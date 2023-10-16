#ifndef MOL_DEF_GUARD_DEFINE_REPRO_CPP_RES_DEF_GUARD_
#define MOL_DEF_GUARD_DEFINE_REPRO_CPP_RES_DEF_GUARD_

#include <set>
#include <map>
#include <list>
#include <ctime>
#include "reprocpp/promise.h"
#include "priocpp/common.h"

namespace prio {

namespace Resource {

	template<class T>
	class InvalidResources
	{
	public:

		static bool is_invalid(T t)
		{
			return invalid_set().count(t) != 0;
		}

		static void invalidate(T t)
		{
			invalid_set().insert(t);
		}

		static void revoke(T t)
		{
			invalid_set().erase(t);
		}

	private:

		static std::set<T>& invalid_set()
		{
			static std::set<T> invalid_;
			return invalid_;
		}
	};

	template<class L>
	class Pool {
	public:

		typedef typename L::type type;
		typedef std::shared_ptr<std::remove_pointer_t<type>> ResourcePtr;
		typedef repro::Promise<ResourcePtr> PromiseType;
		typedef repro::Future<ResourcePtr> FutureType;

		int MAX_AGE;

		Pool()
			: MAX_AGE(20),
			min_capacity_(4)
		{

		}

		Pool(int c)
			: MAX_AGE(20),
			min_capacity_(c)
		{

		}

		~Pool()
		{
			shutdown();
		}

		void shutdown()
		{
			shutdown_ = true;
			for (auto it = unused_.begin(); it != unused_.end(); it++)
			{
				while ((*it).second.size() > 0)
				{
					auto jt = (*it).second.begin();
					L::free(*jt);
					(*it).second.erase(jt);
				}
			}
			for (auto it = used_.begin(); it != used_.end(); it++)
			{
				while ((*it).second.size() > 0)
				{
					auto jt = (*it).second.begin();
					L::free(*jt);
					(*it).second.erase(jt);
				}
			}

			used_.clear();
			unused_.clear();
		}

		FutureType get(const std::string url)
		{
			auto p = repro::Promise<ResourcePtr>();

			if (unused_.count(url) > 0)
			{
				while (!unused_[url].empty())
				{
					auto tmp = *(unused_[url].begin());
					type r = tmp;
					unused_[url].erase(r);

					if (timestamps_[r] + MAX_AGE < unix_timestamp())
					{
						timestamps_.erase(r);
						L::free(r);
						continue;
					}

					timestamps_[r] = unix_timestamp();

					used_[url].insert(r);

					p.resolve(make_ptr(url, r));

					return p.future();
				}
			}

			if (used_[url].size() + pending_ < min_capacity_)
			{
				pending_++;
				L::retrieve(url)
					.then([this, p, url](type r)
						{
							used_[url].insert(r);
							timestamps_[r] = unix_timestamp();

							p.resolve(make_ptr(url, r));
							pending_--;
						})
					.otherwise([this, p](const std::exception_ptr& ex)
						{
							p.reject(ex);
							pending_--;
						});
			}
			else
			{
				waiting_.push_back(p);
			}
			return p.future();
		}


	private:


		void collect(const std::string& url, type t)
		{
			if (shutdown_ == true) return;

			if (InvalidResources<type>::is_invalid(t))
			{
				InvalidResources<type>::revoke(t);
				release(url, t);
				return;
			}

			if (!waiting_.empty())
			{
				PromiseType p = waiting_.front();
				waiting_.pop_front();
				timestamps_[t] = unix_timestamp();
				p.resolve(make_ptr(url, t));
				return;
			}

			if (used_[url].count(t) > 0)
			{
				used_[url].erase(t);
				unused_[url].insert(t);

				while (unused_[url].size() > min_capacity_)
				{
					type tmp = *(unused_[url].begin());
					unused_[url].erase(tmp);
					if (timestamps_.count(tmp) != 0)
					{
						timestamps_.erase(tmp);
					}

					L::free(tmp);
				}
			}
			else
			{
				L::free(t);
			}
		}

		void release(const std::string& url, type t)
		{
			if (shutdown_ == true) return;

			if (used_[url].find(t) != used_[url].end())
			{
				used_[url].erase(t);
			}
			if (timestamps_.count(t) != 0)
			{
				timestamps_.erase(t);
			}
			L::free(t);
		}


		ResourcePtr make_ptr(std::string url, type r)
		{
			return ResourcePtr(
				r,
				[this, url](type t)
				{
					collect(url, t);
				}
			);
		}

		bool shutdown_ = false;
		unsigned int min_capacity_ = 4;
		int pending_ = 0;

		std::map<std::string, std::set<type>> used_;
		std::map<std::string, std::set<type>> unused_;
		std::map<type, long long> timestamps_;
		std::list<PromiseType> waiting_;

	};

	template<class T>
	void invalidate(T* t)
	{
		InvalidResources<T*>::invalidate(t);
	}

	template<class T>
	void invalidate(std::shared_ptr<T>& t)
	{
		invalidate(t.get());
	}

}} // end namespaces

#endif

