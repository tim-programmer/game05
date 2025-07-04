#pragma once

#include <memory>

namespace tr {

// Base class for an object that can be scoped.
struct scoped_object
{
    scoped_object();
    virtual ~scoped_object();
    virtual void apply() = 0;
    virtual void unapply() = 0;
};

//typedef std::unique_ptr<scoped_object> scoped_object_ptr;

class scope
{
public:
    explicit scope(scoped_object& obj);
    ~scope();
private:
    scoped_object& obj_;
    scope() = delete;
    scope(const scope&) = delete;
    scope(scope&&) = delete;
    scope& operator=(const scope&) = delete;
    scope& operator=(scope&&) = delete;
};

}