
#ifndef INTERFACE_RUNTIME_ABS_TIME_H_
#define INTERFACE_RUNTIME_ABS_TIME_H_

namespace fasmio { namespace runtime_env {

class TimeSpan
{
public:
    TimeSpan();
    TimeSpan(long seconds, long useconds = 0);
    TimeSpan(const TimeSpan &);

public:
    TimeSpan& operator= (const TimeSpan &);

    TimeSpan operator+ () const;
    TimeSpan operator- () const;

    TimeSpan operator+ (const TimeSpan &) const;
    TimeSpan operator- (const TimeSpan &) const;
    TimeSpan& operator+= (const TimeSpan &);
    TimeSpan& operator-= (const TimeSpan &);

    bool operator<  (const TimeSpan &) const;
    bool operator<= (const TimeSpan &) const;
    bool operator>  (const TimeSpan &) const;
    bool operator>= (const TimeSpan &) const;
    bool operator== (const TimeSpan &) const;
    bool operator!= (const TimeSpan &) const;

public:
    TimeSpan& Adjust(long seconds = 0, long useconds = 0);

public:
    long seconds () const { return seconds_;  }
    long useconds() const { return useconds_; }

private:
    long seconds_;
    long useconds_;
};


class ABSTime
{
public:
    ABSTime();
    ABSTime(unsigned long seconds, unsigned long useconds = 0);
    ABSTime(const ABSTime &);

public:
    ABSTime& operator= (const ABSTime &);

    ABSTime operator+ (const TimeSpan &) const;
    ABSTime operator- (const TimeSpan &) const;
    ABSTime& operator+= (const TimeSpan &);
    ABSTime& operator-= (const TimeSpan &);

    TimeSpan operator- (const ABSTime &) const;

    bool operator<  (const ABSTime &) const;
    bool operator<= (const ABSTime &) const;
    bool operator>  (const ABSTime &) const;
    bool operator>= (const ABSTime &) const;
    bool operator== (const ABSTime &) const;
    bool operator!= (const ABSTime &) const;

public:
    ABSTime& Now();
    ABSTime& Adjust(const TimeSpan &);
    ABSTime& Adjust(long seconds = 0, long useconds = 0);

public:
    unsigned long seconds () const { return seconds_;  }
    unsigned long useconds() const { return useconds_; }

private:
    unsigned long seconds_;
    unsigned long useconds_;
};

}}  // namespace fasmio::runtime_env

#endif  // INTERFACE_RUNTIME_ABS_TIME_H_

