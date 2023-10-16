#ifndef _MOL_DEF_GUARD_METRICS_PROMETHEUS_DEFINE_
#define _MOL_DEF_GUARD_METRICS_PROMETHEUS_DEFINE_

#include "reproweb/ctrl/controller.h"
#include "reproweb/view/i18n.h"
#include "reproweb/web_webserver.h"
#include <signal.h>
  
namespace Prometheus
{


using Label = std::pair<std::string,std::string>;


class Metric
{
public:

	Metric() {}

	Metric(std::string n)
		: name(n)
	{}

	template<class ... Args>
	Metric(std::string n, Args ... args)
		: name(n)
	{
		std::vector<Label> labels{ args ... };

		sort(labels.begin(), labels.end(), [](Label lhs, Label rhs) 
		{ 
			if(lhs.first != rhs.first) 
			{
				return lhs.first < rhs.first;
			}
			return lhs.second < rhs.second;
		});

		std::ostringstream oss;
		oss << name << "{";

		if( !labels.empty() )
		{
			auto it = labels.begin();
			write_label(oss,*it);
			it++;

			while( it != labels.end() )
			{
				oss << ",";
				write_label(oss,*it);
				it++;
			}
		}

		oss << "}";
		path = oss.str();
	}

	std::string name;
	std::string path;

private:

	void write_label(std::ostringstream& oss, const Label& label)
	{
		oss << label.first << "=\"" << label.second << "\"";
	}
};

class Counter : public Metric
{
public:

	Counter()
		: value_(0)
	{}

	Counter(const Metric& m)
		: Metric(m), value_(0)
	{}

	void record(unsigned int delta = 1)
	{
		value_ += delta;
	}

	int value()
	{
		return value_;
	}

	void flush(std::ostringstream& oss)
	{
		oss << "# TYPE " << name << " counter" << "\n";
		oss << path << " " << value() << "\n";
	}

	void reset()
	{
		value_ = 0;
	}

private:
	int value_;
};


class Gauge : public Metric
{
public:

	Gauge()
		: value_(0), count_(0)
	{}

	Gauge(const Metric& m)
		: Metric(m), value_(0), count_(0)
	{}

	void record( double delta = 1)
	{
		value_ += delta;
		count_++;
	}

	double value()
	{
		return value_ / count_;
	}

	void flush(std::ostringstream& oss)
	{
		oss << "# TYPE " << name << " gauge" << "\n";
		oss << path << " " << value() << "\n";
	}

	void reset()
	{
		value_ = 0;
		count_ = 0;
	}

private:
	double value_;
	unsigned int count_; 
};

class Collector
{
public:

	void increment(const Metric& metric, unsigned int delta = 1)
	{
		if(counters_.count(metric.path) == 0)
		{
			counters_[metric.path] = Counter(metric);			
		}
		counters_[metric.path].record(delta);
	}

	void record(const Metric& metric, unsigned int val = 1)
	{
		if(gauges_.count(metric.path) == 0)
		{
			gauges_[metric.path] = Gauge(metric);			
		}
		gauges_[metric.path].record(val);
	}

	std::string flush()
	{
		std::ostringstream oss;
		for( auto& c : counters_)
		{
			c.second.flush(oss);
		}
		for( auto& g : gauges_)
		{
			g.second.flush(oss);
		}
		return oss.str();
	}

	void clear()
	{
		for( auto& c : counters_)
		{
			c.second.reset();
		}
		for( auto& g : gauges_)
		{
			g.second.reset();
		}
	}

private:

	std::map<std::string,Counter> counters_;
	std::map<std::string,Gauge> gauges_;
};

class Controller
{
public:

	Controller(std::shared_ptr<Collector> c)
		: collector_(c)
	{}

	void metrics(::prio::Request& req, ::prio::Response& res)
	{
		std::string stats = collector_->flush();
		res.contentType("text/plain").body(stats).ok().flush();
	}

private:

	std::shared_ptr<Collector> collector_;
};

} // end namespace prometheus

#endif
