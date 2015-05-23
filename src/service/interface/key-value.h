
#ifndef SERVICE_INTERFACE_KEY_VALUE_H_
#define SERVICE_INTERFACE_KEY_VALUE_H_

namespace fasmio { namespace service {

class IKey;

class IValue
{
public:
    virtual ~IValue() {};

public:
    virtual IValue*       Clone                () = 0;

    virtual const char*   GetName              () = 0;
    virtual bool          SetName              (const char*) = 0;

    virtual const char*   GetTextValue         () = 0;
    virtual long          GetLongValue         () = 0;
    virtual double        GetDoubleValue       () = 0;
    virtual bool          GetBoolValue         () = 0;

    virtual bool          TryGetLongValue      (long   *value) = 0;
    virtual bool          TryGetDoubleValue    (double *value) = 0;
    virtual bool          TryGetBoolValue      (bool   *value) = 0;

    virtual bool          SetTextValue         (const char* value) = 0;
    virtual bool          SetLongValue         (long        value) = 0;
    virtual bool          SetDoubleValue       (double      value) = 0;
    virtual bool          SetBoolValue         (bool        value) = 0;

    virtual IValue*       GetNextValue         () = 0;
    virtual IValue*       GetPrevValue         () = 0;
    virtual IKey*         GetParentKey         () = 0;
};

class IKey
{
public:
    virtual ~IKey() {};

public:
    virtual IKey*         Clone                () = 0;
    virtual void          Clear                () = 0;
    virtual unsigned long ChildrenCount        () = 0;
    virtual unsigned long SubKeyCount          () = 0;
    virtual unsigned long ValueCount           () = 0;

    virtual const char*   GetName              () = 0;
    virtual bool          SetName              (const char*) = 0;

    virtual IKey*         GetParentKey         () = 0;
    virtual IKey*         GetNextKey           () = 0;
    virtual IKey*         GetPrevKey           () = 0;
    virtual IKey*         GetFirstChildKey     () = 0;
    virtual IKey*         GetLastChildKey      () = 0;
    virtual IKey*         GetKey               (const char* name) = 0;

    virtual IKey*         AppendKey            (const char* name) = 0;
    virtual IKey*         ReleaseKey           (const char* name) = 0;
    virtual bool          DeleteKey            (const char* name) = 0;
    virtual IKey*         AppendKey            (IKey* key) = 0;
    virtual IKey*         ReleaseKey           (IKey* key) = 0;
    virtual bool          DeleteKey            (IKey* key) = 0;

    virtual IValue*       GetValue             (const char* name) = 0;
    virtual IValue*       GetFirstValue        () = 0;
    virtual IValue*       GetLastValue         () = 0;

    virtual bool          DeleteValue          (const char* name) = 0;
    virtual bool          DeleteValue          (IValue*) = 0;

    virtual const char*   GetTextValue         (const char* name) = 0;
    virtual long          GetLongValue         (const char* name) = 0;
    virtual double        GetDoubleValue       (const char* name) = 0;
    virtual bool          GetBoolValue         (const char* name) = 0;

    virtual bool          TryGetLongValue      (const char* name, long   *value) = 0;
    virtual bool          TryGetDoubleValue    (const char* name, double *value) = 0;
    virtual bool          TryGetBoolValue      (const char* name, bool   *value) = 0;

    virtual bool          SetTextValue         (const char* name, const char*  value, bool auto_add) = 0;
    virtual bool          SetLongValue         (const char* name, long         value, bool auto_add) = 0;
    virtual bool          SetDoubleValue       (const char* name, double       value, bool auto_add) = 0;
    virtual bool          SetBoolValue         (const char* name, bool         value, bool auto_add) = 0;

    virtual bool          AppendTextValue      (const char* name, const char*  value) = 0;
    virtual bool          AppendLongValue      (const char* name, long         value) = 0;
    virtual bool          AppendDoubleValue    (const char* name, double       value) = 0;
    virtual bool          AppendBoolValue      (const char* name, bool         value) = 0;
};

}}  // namespace fasmio::service

#endif // SERVICE_INTERFACE_KEY_VALUE_H_

