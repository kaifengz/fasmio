
#include "interface/runtime-env/abs-time.h"

#include <stdlib.h>
#include <sys/time.h>

namespace fasmio { namespace runtime_env {

template <typename Type>
static void normalize(Type &seconds, Type &useconds)
{
    const long kMillion = 1000 * 1000;

    if (useconds < 0)
    {
        long sec_adjustment = (-useconds) / kMillion + 1;
        seconds  -= sec_adjustment;
        useconds += sec_adjustment * kMillion;
    }

    if (static_cast<long>(useconds) >= kMillion)
    {
        seconds += useconds / kMillion;
        useconds = useconds % kMillion;
    }
}



TimeSpan::TimeSpan() :
    seconds_(0), useconds_(0)
{
}

TimeSpan::TimeSpan(long seconds, long useconds) :
    seconds_(seconds), useconds_(useconds)
{
    normalize(seconds_, useconds_);
}

TimeSpan::TimeSpan(const TimeSpan &another) :
    seconds_(another.seconds_), useconds_(another.useconds_)
{
}

TimeSpan& TimeSpan::operator= (const TimeSpan &another)
{
    seconds_  = another.seconds_;
    useconds_ = another.useconds_;
    return *this;
}

TimeSpan TimeSpan::operator+ () const
{
    return *this;
}

TimeSpan TimeSpan::operator- () const
{
    return TimeSpan(-seconds_, -useconds_);
}

TimeSpan TimeSpan::operator+ (const TimeSpan &another) const
{
    return TimeSpan(seconds_ + another.seconds_, useconds_ + another.useconds_);
}

TimeSpan TimeSpan::operator- (const TimeSpan &another) const
{
    return TimeSpan(seconds_ - another.seconds_, useconds_ - another.useconds_);
}

TimeSpan& TimeSpan::operator+= (const TimeSpan &another)
{
    seconds_  += another.seconds_;
    useconds_ += another.useconds_;
    normalize(seconds_, useconds_);
    return *this;
}

TimeSpan& TimeSpan::operator-= (const TimeSpan &another)
{
    seconds_  -= another.seconds_;
    useconds_ -= another.useconds_;
    normalize(seconds_, useconds_);
    return *this;
}

bool TimeSpan::operator<  (const TimeSpan &another) const
{
    return seconds_ < another.seconds_ ? true :
           seconds_ > another.seconds_ ? false :
           useconds_ < another.useconds_;
}

bool TimeSpan::operator<= (const TimeSpan &another) const
{
    return !(another < *this);
}

bool TimeSpan::operator>  (const TimeSpan &another) const
{
    return another < *this;
}

bool TimeSpan::operator>= (const TimeSpan &another) const
{
    return !(*this < another);
}

bool TimeSpan::operator== (const TimeSpan &another) const
{
    return seconds_ == another.seconds_ && useconds_ == another.useconds_;
}

bool TimeSpan::operator!= (const TimeSpan &another) const
{
    return !(*this == another);
}

TimeSpan& TimeSpan::Adjust(long seconds, long useconds)
{
    seconds_  += seconds;
    useconds_ += useconds_;
    normalize(seconds_, useconds_);
    return *this;
}



ABSTime::ABSTime()
{
    Now();
}

ABSTime::ABSTime(unsigned long seconds, unsigned long useconds) :
    seconds_(seconds), useconds_(useconds)
{
    normalize(seconds_, useconds_);
}

ABSTime::ABSTime(const ABSTime &time) :
    seconds_(time.seconds_), useconds_(time.useconds_)
{
}

ABSTime& ABSTime::operator= (const ABSTime &time)
{
    seconds_  = time.seconds_;
    useconds_ = time.useconds_;
    return *this;
}

ABSTime ABSTime::operator+ (const TimeSpan &span) const
{
    return ABSTime(seconds_ + span.seconds(), useconds_ + span.useconds());
}

ABSTime ABSTime::operator- (const TimeSpan &span) const
{
    return ABSTime(seconds_ - span.seconds(), useconds_ - span.useconds());
}

ABSTime& ABSTime::operator+= (const TimeSpan &span)
{
    seconds_  += span.seconds();
    useconds_ += span.useconds();
    normalize(seconds_, useconds_);
    return *this;
}

ABSTime& ABSTime::operator-= (const TimeSpan &span)
{
    seconds_  -= span.seconds();
    useconds_ -= span.useconds();
    normalize(seconds_, useconds_);
    return *this;
}

TimeSpan ABSTime::operator- (const ABSTime &another) const
{
    return TimeSpan(seconds_ - another.seconds_, useconds_ - another.useconds_);
}

bool ABSTime::operator<  (const ABSTime &time) const
{
    return seconds_ < time.seconds_ ? true :
           seconds_ > time.seconds_ ? false :
           useconds_ < time.useconds_;
}

bool ABSTime::operator<= (const ABSTime &time) const
{
    return !(time < *this);
}

bool ABSTime::operator>  (const ABSTime &time) const
{
    return time < *this;
}

bool ABSTime::operator>= (const ABSTime &time) const
{
    return !(*this < time);
}

bool ABSTime::operator== (const ABSTime &time) const
{
    return seconds_ == time.seconds_ && useconds_ == time.useconds_;
}

bool ABSTime::operator!= (const ABSTime &time) const
{
    return !(*this == time);
}

ABSTime& ABSTime::Now()
{
    struct timeval now;
    gettimeofday(&now, nullptr);
    seconds_  = now.tv_sec;
    useconds_ = now.tv_usec;
    return *this;
}

ABSTime& ABSTime::Adjust(const TimeSpan &span)
{
    return Adjust(span.seconds(), span.useconds());
}

ABSTime& ABSTime::Adjust(long seconds, long useconds)
{
    seconds_ += seconds;
    useconds_ += useconds;
    normalize(seconds_, useconds_);
    return *this;
}

}}  // namespace fasmio::runtime_env

