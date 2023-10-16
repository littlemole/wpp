#include "reproweb/tools/metrics.h"

#include <numeric>

namespace reproweb  {



Metric::Metric()
{}

Metric::Metric(const std::string& k)
	: key(k)
{}

size_t Metric::count()
{
	return values.size();
}

size_t Metric::avg()
{
	if(values.empty())
	{
		return 0;
	}
	return (size_t)std::accumulate(values.begin(), values.end(), 0.0) / values.size();
}

void Metric::clear()
{
	values.clear();
}

size_t Metric::sum()
{
	return std::accumulate(values.begin(), values.end(), 0);
}


MetricsCollector::MetricsCollector(
		const std::string& graphite_host,
		int graphite_port,
		int frequency
	)
	: graphite_host_(graphite_host),
	  graphite_port_(graphite_port),
	  frequency_(frequency)
{
	prio::TcpConnection::connect( graphite_host_, graphite_port_)
	.then([this](prio::ConnectionPtr ptr)
	{
		con_ = ptr;
	})
	.otherwise( [](const std::exception& ex)
	{
		std::cout << ex.what() << std::endl;
	});
}


void MetricsCollector::timer()
{
	if(con_)
	{
		std::ostringstream oss;

		for( auto& it : metrics_)
		{
			std::string key = it.first->key;
			size_t val = (it.first->*(it.second))();

			it.first->clear();

			oss << key << " " << val << " " << ts() << "\r\n\r\n";
		}
		std::cout << oss.str() << std::endl;
		con_->write(oss.str());
		// TODO: err handling, reconnect
	}

	timer_.after( [this](){ timer(); }, frequency_*1000);
/*
	org::timeout([this]()
	{
		timer();
	},frequency_,0);
*/	
}

std::time_t  MetricsCollector::ts()
{
	 std::time_t result = std::time(nullptr);
	 return result;
}


}
