
#ifndef INCLUDE_PROMISE_WEB_METRICS_H_
#define INCLUDE_PROMISE_WEB_METRICS_H_

#include <ctime>
#include <sstream>
#include <iostream>
#include <priocpp/connection.h>
#include <priocpp/timeout.h>


namespace reproweb  {


class Metric
{
public:

	Metric();
	Metric(const std::string& k);

	std::string key;
	std::vector<int> values;

	size_t sum();
	size_t count();
	size_t avg();

	void clear();
};

class MetricsCollector
{
public:

	MetricsCollector(
		const std::string& graphite_host,
		int graphite_port,
		int frequency
	);

	void timer();
	static std::time_t  ts();

protected:

	std::string graphite_host_;
	int graphite_port_;
	int frequency_;

	typedef size_t (Metric::*fp)();


	template<class T>
	void add_metric(T& metric, fp fun)
	{
		metrics_.push_back(std::pair<Metric*,fp>(metric.get(),fun));
	}

	std::vector<std::pair<Metric*,fp>> metrics_;
	prio::ConnectionPtr con_;
	prio::Timeout timer_;
};

}


#endif /* INCLUDE_PROMISE_WEB_METRICS_H_ */
