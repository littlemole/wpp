#ifndef MOL_DEF_GUARD_DEFINE_REPRO_CPP_CALLBACK_DEF_GUARD_
#define MOL_DEF_GUARD_DEFINE_REPRO_CPP_CALLBACK_DEF_GUARD_

#include "reprocpp/promise_mixin.h"

namespace prio {

template<class ... Args>
class Callback
{
public:

    template<class F>
    Callback& then(F&& f)
    {
        cb_ = f;
        return *this;
    }

    template<class E>
    Callback& otherwise(E&& e)
    {
        repro::otherwise_chain(err_,e);
        return *this;
    }

    template<class ...VArgs>
    void resolve(VArgs&& ... args)
    {
        cb_(std::forward<VArgs>(args)...);
    }

    void reject(const std::exception_ptr& eptr) const
    {
        if (err_)
        {

            try
            {
                err_(eptr);
            }
            catch (...)
            {
                throw;
            }
        }
    }    

    void reject(std::exception_ptr& eptr) const
    {
        if (err_)
        {

            try
            {
                err_(eptr);
            }
            catch (...)
            {
                throw;
            }
        }
    }  

    template<class E>
    void reject(E&& e) const
    {
        auto eptr = std::make_exception_ptr(e);
        reject(eptr);
    }

private:
    std::function<void(Args...)> cb_;
    std::function<bool(std::exception_ptr)> err_;
};


} // end namespace repro


#endif