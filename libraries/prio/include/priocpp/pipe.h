#ifndef MOL_PROMISE_LIBEVENT_PipedProcess_DEF_GUARD_DEFINE_
#define MOL_PROMISE_LIBEVENT_PipedProcess_DEF_GUARD_DEFINE_

/**
 * \file PipedProcess.h
 */

#ifndef _WIN32

#include "reprocpp/promise.h"
#include "priocpp/api.h"
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
namespace prio      {

void set_blocking(socket_t fd);	

class Pipe: public std::enable_shared_from_this<Pipe>
{
public:

	~Pipe()
	{
			REPRO_MONITOR_DECR(Pipe);
	}

	static std::shared_ptr<Pipe> create()
	{
		auto ptr = std::shared_ptr<Pipe>(new Pipe());
		ptr->pipe();

		return ptr;
	}

	void close()
	{
		io_write_.cancel();
		io_read_.cancel();
		closeFd(0);
		closeFd(1);

		if(t_)
		{
			t_->join();
			t_.reset();
		}
	}

	repro::Future<> write(std::string data)
	{
		auto p = repro::promise();

		int r = ::write(filedes_[1],data.c_str(),data.size());
		if( r == EWOULDBLOCK || r == EAGAIN)
		{
			io_write_
			.onWrite(filedes_[1])
			.then([this,p,data]()
			{
				this->write(data)
				.then([p]()
				{
					p.resolve();
				})
				.otherwise(reject(p));
			})
			.otherwise(reject(p));
			return  p.future();
		}

		if( ((unsigned int)r) == data.size())
		{
			nextTick([p]()
			{
				p.resolve();
			});
		}
		else
		{
			this->write(data.substr(r))
			.then([p]()
			{
				p.resolve();
			})
			.otherwise(reject(p));
		}

		return p.future();
	}

	void asyncReader( std::function<void(std::string)> fun)
	{
		std::vector<std::function<void(std::string)>> f{fun};
		asyncReader(f);
	}

	void asyncReader( std::vector<std::function<void(std::string)>> fun)
	{
		set_blocking(filedes_[0]);

		t_.reset( new std::thread( [this,fun]()
		{
			while(true)
			{
				char buf[1024];
				int n = ::read(filedes_[0],buf,1024);
				if(n>0)
				{
					std::string tmp(buf,n);
					for( auto& f: fun)
					{
						f(tmp);
					}
				}
				if(n<1)
				{
					break;
				}
			}
		}) );
	}

	void asyncLineReader( std::function<void(std::string)> fun)
	{
		std::vector<std::function<void(std::string)>> f{fun};
		asyncLineReader(f);
	}

	void asyncLineReader( std::vector<std::function<void(std::string)>> fun)
	{
		set_blocking(filedes_[0]);

		t_.reset( new std::thread( [this,fun]()
		{
			std::string line_buf;

			while(true)
			{
				char buf[1024];
				int n = ::read(filedes_[0],buf,1024);
				if(n>0)
				{
					std::string tmp(buf,n);
					line_buf.append(tmp);

					auto pos = line_buf.find("\n");

					while(pos!=std::string::npos)
					{
						std::string line = line_buf.substr(0,pos);
						if(pos+1>line_buf.size())
						{
							line_buf = "";
						}
						else 
						{
							line_buf = line_buf.substr(pos+1);
						}

						for(auto& f : fun)
						{
							f(line);
						}

						pos = line_buf.find("\n");
					}					
					continue;
				}
				if(n<1)
				{
					break;
				}
			}
		}) );
	}

	repro::Future<std::string> read()
	{
		auto p =repro::promise<std::string>();

		io_read_
		.onRead(filedes_[0])
		.then([this,p]()
		{
			char buf[1024];
			int n = ::read(filedes_[0],buf,1024);
			p.resolve(std::string(buf,n));
		})
		.otherwise(reject(p));

		return p.future();
	}	
	

private:

	void closeFd(int i)
	{
		if(filedes_[i] != -1)
		{
			::close(filedes_[i]);
			filedes_[i] = -1;
		}
	}

	void readLine(repro::Promise<std::string> p)
	{
		io_read_.cancel();

		auto pos = line_buffer_.find("\n");
		if(pos!=std::string::npos)
		{
			std::string line = line_buffer_.substr(0,pos);
			if(pos+1>line_buffer_.size())
			{
				line_buffer_ = "";
			}
			else 
			{
				line_buffer_ = line_buffer_.substr(pos+1);
			}
			nextTick([p,line]()
			{
				p.resolve(line);				
			});
			return;
		}

		io_read_
		.onRead(filedes_[0])
		.then([this,p]()
		{
			char buf[1024];
			int n = ::read(filedes_[0],buf,1024);
			line_buffer_.append(std::string(buf,n));

			readLine(p);
		})
		.otherwise(reject(p));

		return;
	}	

public:


	repro::Future<std::string> readLine()
	{
		auto p = repro::promise<std::string>();

		readLine(p);

		return p.future();
	}

private:

	std::unique_ptr<std::thread> t_;
	std::string line_buffer_;

	IO io_write_;
	IO io_read_;

	int filedes_[2];

	Pipe()
	{
		filedes_[0] = -1;
		filedes_[1] = -1;
		REPRO_MONITOR_INCR(Pipe);
	}

	Pipe& operator=(const Pipe& rhs) = delete;
	Pipe(const Pipe& rhs) = delete;

	void pipe()
	{
		if (pipe2(filedes_, O_NONBLOCK|O_CLOEXEC) == -1)
		{
			throw repro::Ex("create Pipe failed");
		}		
	}
};


/**
 * \brief unix PipedProcess implementation 
 *
 * open a process and control its input and output asynchronously.
 **/
class PipedProcess : public std::enable_shared_from_this<PipedProcess>
{
public:

	//! a PipedProcess::Ptr is a std::shared_ptr<PipedProcess>
	typedef std::shared_ptr<PipedProcess> Ptr;


	template<class ... Args>
	class Arguments
	{
	public:
		Arguments(Args ... args)
			: args_{args...,(const char*)NULL}
		{
		}

		std::vector<const char*> get() const
		{
			return args_;
		}

	private:
		std::vector<const char*> args_;
	};


	template<class ... Args>
	static auto arguments(Args ... args)
	{
		return Arguments<Args...>(args...);
	}

	template<class ... Args>
	static auto environment(Args ... args)
	{
		return Arguments<Args...>(args...);
	}	

	PipedProcess();
	~PipedProcess();

	//! create a PipedProcess as a shared ptr
	static Ptr create();

	//! specify stdin for PipedProcessd process
	Ptr putStdin(const std::string& s);

	//! specify path for subprocess to execute
	//! once the PipedProcessd process has finished, the future will be resolved
	repro::Future<PipedProcess::Ptr> pipe(const std::string& path )
	{
		return PipedProcess_impl( path );
	}


	//! specify path for subprocess to execute and add arguments
	//! once the PipedProcessd process has finished, the future will be resolved
	template<class A>
	repro::Future<PipedProcess::Ptr> pipe(const std::string& path, A&& a )
	{
		args_ = a.get();
		return PipedProcess_impl( path, ( char* const*) &(args_[0]) );
	}

	//! specify path for subprocess to execute, add arguments and specify env** vector
	//! once the PipedProcessd process has finished, the future will be resolved
	template<class A>
	repro::Future<PipedProcess::Ptr> pipe(const std::string& path, A&& a, char ** env )
	{
		args_ = a.get();
		return PipedProcess_impl( path, ( char* const*) &(args_[0]), env );
	}

	//! specify path for subprocess to execute, add arguments and specify environment as std::vector
	//! once the PipedProcessd process has finished, the future will be resolved
	template<class A, class E>
	repro::Future<PipedProcess::Ptr> pipe(const std::string& path, A&& args, E&& env )
	{
		args_ = args.get();
		env_ = env.get();
		return PipedProcess_impl( path,  (char* const*) &(args_[0]),  (char* const*) &(env_[0]) );
	}

	//! get the PipedProcessd processes stdout
	std::string getStdout();
	//! get the PipedProcessd processes stderr
	std::string getStderr();

	//! exit code from PipedProcessd process
	int result();

private:

	repro::Future<PipedProcess::Ptr> PipedProcess_impl(const std::string& path,   char* const* args = NULL,  char* const* env = NULL);

	void run_child(const std::string& path,  char* const* args,  char* const* env);

	void run_parent();

	repro::Future<> read(repro::Promise<>);
	repro::Future<> write(repro::Promise<>);


	int result_;
	pid_t pid_;
	int filedes_[2];
	std::ostringstream stdout_oss_;
	std::ostringstream stderr_oss_;
	std::string stdin_;
	size_t written_;

	std::vector<const char*> args_;
	std::vector<const char*> env_;

	IO io_;
	std::shared_ptr<PipedProcess> self_;
	repro::Promise<PipedProcess::Ptr> promise_;
};


}

#endif
#endif
