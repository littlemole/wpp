#include <functional>

#include "priocpp/pipe.h"
#include "priocpp/loop.h"

#ifndef _WIN32

#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>

using namespace repro;

namespace prio      {


void set_blocking(socket_t fd)
{
#ifndef _WIN32
	// set non blocking
	int flags;
	if ( -1 == (flags = fcntl(fd, F_GETFL, 0)) )
	{
		flags = 0;        
	}
	flags  &= ~O_NONBLOCK;
	fcntl(fd, F_SETFL, flags);  
#else
	u_long iMode = 0;
	ioctlsocket(fd, FIONBIO, &iMode);
#endif
}


PipedProcess::PipedProcess()
	: result_(0), pid_(0),written_(0), promise_(promise<PipedProcess::Ptr>())
{
	REPRO_MONITOR_INCR(PipedProcess);
}

PipedProcess::~PipedProcess()
{
	REPRO_MONITOR_DECR(PipedProcess);
}


PipedProcess::Ptr PipedProcess::create()
{
	auto ptr = std::make_shared<PipedProcess>();
	ptr->self_ = ptr;
	return ptr;
}

PipedProcess::Ptr PipedProcess::putStdin(const std::string& s)
{
	stdin_ = s;
	return self_;
}


Future<PipedProcess::Ptr> PipedProcess::PipedProcess_impl(const std::string& path, char* const* args, char* const* env)
{
	if (pipe2(filedes_, O_NONBLOCK|O_CLOEXEC) == -1)
	{
		self_.reset();
		throw Ex("create PipedProcess failed");
	}
	pid_ = fork();
	if (pid_ == -1)
	{
		self_.reset();
		throw Ex("fork failed");
	}
	else if (pid_ == 0)
	{
		run_child(path,args,env);
	}
	else
	{
		run_parent();
	}

	return promise_.future();
}

std::string PipedProcess::getStdout()
{
	return stdout_oss_.str();
}

std::string PipedProcess::getStderr()
{
	return stderr_oss_.str();
}

int PipedProcess::result()
{
	return result_;
}


void PipedProcess::run_child(const std::string& path, char* const* args, char* const* env)
{
	  while ((dup2(filedes_[0], STDIN_FILENO)  == -1) && (errno == EINTR)) {}
	  while ((dup2(filedes_[1], STDOUT_FILENO) == -1) && (errno == EINTR)) {}
	  close(filedes_[0]);
	  close(filedes_[1]);
	  close(STDIN_FILENO);
	  close(STDERR_FILENO);

	  execve(path.c_str(), args, env);

	  throw Ex("execl child failed");
}

void PipedProcess::run_parent()
{
	written_ = 0;

	auto pw = promise<>();
	auto pr = promise<>();

	write(pw)
	.then( [this,pr] ()
	{
		close(filedes_[1]);
		return read(pr);
	})
	.then( [this] ()
	{
		promise_.resolve(self_);
		self_.reset();
	})
	.otherwise([this](const std::exception& ex)
	{
		promise_.reject(ex);
		self_.reset();
	});
}

Future<> PipedProcess::read(repro::Promise<> p)
{
	io_.onRead((socket_t)(filedes_[0]))
	.then([this,p]()
	{
		while(true)
		{
			char buf[1024];
			auto len = ::read(filedes_[0],buf,1024);

			if(len > 0)
			{
				stdout_oss_.write(buf,len);
				continue;
			}
			else if(len<=0)
			{
				if( (len == -1) && (errno == EWOULDBLOCK) )
				{
					this->read(p);
				}
				else
				{
					waitpid(this->pid_, NULL, 0);

					close(this->filedes_[0]);

					p.resolve();
				}
			}
			break;
		}

	});

	return p.future();
}

Future<> PipedProcess::write(repro::Promise<> p)
{
	if(stdin_.empty())
	{
		auto p = promise();
		nextTick([p](){p.resolve();});
		return p.future();
	}

	io_.onWrite(filedes_[1])
	.then([this,p]()
	{
		while(true)
		{
			auto len = ::write(filedes_[1],this->stdin_.c_str()+this->written_, this->stdin_.size()-this->written_);

			if(len > 0)
			{
				this->written_ += len;
				if ( this->written_ >= this->stdin_.size() )
				{
					p.resolve();
					break;
				}
				continue;
			}
			else if(len<=0)
			{
				if( (len == -1) && (errno == EWOULDBLOCK) )
				{
					this->write(p);
				}
				else
				{
					waitpid(this->pid_, NULL, 0);

					close(this->filedes_[0]);
					close(this->filedes_[1]);

					p.reject(Ex("IoEx in PipedProcess::write"));
				}
			}
			break;
		}
	});

	return p.future();
}


}

#endif
