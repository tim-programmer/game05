#include "tr_scope.h"

namespace tr
{

scoped_object::scoped_object()
{
}

scoped_object::~scoped_object()
{
}

scope::scope(scoped_object& obj)
    : obj_(obj)
{
    obj_.apply();
}

scope::~scope()
{
    obj_.unapply();
}

} // namespace tr
