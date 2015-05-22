
#ifndef CONTAINER_KEY_VALUE_IMPL_H_
#define CONTAINER_KEY_VALUE_IMPL_H_

#include "service/interface/key-value.h"
#include "./tlv-composer.h"
#include "./tlv-parser.h"
#include <string>

namespace fasmio { namespace container {

class ValueBase: public fasmio::service::IValue
{
public:
    virtual IValue* Clone()
    {
        return CloneValue();
    }

    virtual IValue* CloneValue() = 0;
};

class KeyBase: public fasmio::service::IKey
{
public:
    virtual IKey* Clone()
    {
        return CloneKey();
    }

    virtual IKey* CloneKey() = 0;
};

enum KeyValueType
{
    KeyValue_Undecided = 0,
    KeyValue_Key,
    KeyValue_Value,
};

class KeyValueImpl: public ValueBase, public KeyBase
{
public:
    KeyValueImpl();
    virtual ~KeyValueImpl();

public:
    // non-virtual
    bool                  Serialize            (TlvComposer*);
    bool                  UnSerialize          (TlvParser*);

    void                  Swap                 (KeyValueImpl &);

public:
    // for IValue & IKey common
    virtual const char*   GetName              ();
    virtual bool          SetName              (const char*);

    virtual IKey*         GetParentKey         ();

public:
    // for IValue only
    virtual IValue*       CloneValue           ();

    virtual const char*   GetTextValue         ();
    virtual long          GetLongValue         ();
    virtual double        GetDoubleValue       ();
    virtual bool          GetBoolValue         ();

    virtual bool          TryGetLongValue      (long   *value);
    virtual bool          TryGetDoubleValue    (double *value);
    virtual bool          TryGetBoolValue      (bool   *value);

    virtual bool          SetTextValue         (const char* value);
    virtual bool          SetLongValue         (long        value);
    virtual bool          SetDoubleValue       (double      value);
    virtual bool          SetBoolValue         (bool        value);

    virtual IValue*       GetNextValue         ();
    virtual IValue*       GetPrevValue         ();

public:
    // for IKey only
    virtual IKey*         CloneKey             ();

    virtual void          Clear                ();

    virtual IKey*         GetNextKey           ();
    virtual IKey*         GetPrevKey           ();
    virtual IKey*         GetFirstChildKey     ();
    virtual IKey*         GetLastChildKey      ();
    virtual IKey*         GetKey               (const char* name);

    virtual IKey*         AppendKey            (const char* name);
    virtual IKey*         ReleaseKey           (const char* name);
    virtual bool          DeleteKey            (const char* name);
    virtual IKey*         AppendKey            (IKey* key);
    virtual IKey*         ReleaseKey           (IKey* key);
    virtual bool          DeleteKey            (IKey* key);

    virtual IValue*       GetValue             (const char* name);
    virtual IValue*       GetFirstValue        ();
    virtual IValue*       GetLastValue         ();

    virtual bool          DeleteValue          (const char* name);
    virtual bool          DeleteValue          (IValue*);

    virtual const char*   GetTextValue         (const char* name);
    virtual long          GetLongValue         (const char* name);
    virtual double        GetDoubleValue       (const char* name);
    virtual bool          GetBoolValue         (const char* name);

    virtual bool          TryGetLongValue      (const char* name, long   *value);
    virtual bool          TryGetDoubleValue    (const char* name, double *value);
    virtual bool          TryGetBoolValue      (const char* name, bool   *value);

    virtual bool          SetTextValue         (const char* name, const char*  value, bool auto_add);
    virtual bool          SetLongValue         (const char* name, long         value, bool auto_add);
    virtual bool          SetDoubleValue       (const char* name, double       value, bool auto_add);
    virtual bool          SetBoolValue         (const char* name, bool         value, bool auto_add);

    virtual bool          AppendTextValue      (const char* name, const char*  value);
    virtual bool          AppendLongValue      (const char* name, long         value);
    virtual bool          AppendDoubleValue    (const char* name, double       value);
    virtual bool          AppendBoolValue      (const char* name, bool         value);

protected:
    KeyValueImpl*         CloneKeyValue        ();

    KeyValueImpl*         GetFirstChild        (KeyValueType);
    KeyValueImpl*         GetLastChild         (KeyValueType);
    KeyValueImpl*         GetNextSibling       (KeyValueType);
    KeyValueImpl*         GetPrevSibling       (KeyValueType);

    KeyValueImpl*         GetDeepChild         (const char* name, KeyValueType, bool auto_add);
    KeyValueImpl*         AppendDeepChild      (const char* name, KeyValueType);
    KeyValueImpl*         ReleaseDeepChild     (const char* name, KeyValueType);
    KeyValueImpl*         ReleaseDeepChild     (KeyValueImpl*);

    KeyValueImpl*         AppendGetDeepChild   (char* name, KeyValueType, bool append, bool auto_add = false);
    KeyValueImpl*         GetDirectChild       (const char* name, KeyValueType);
    KeyValueImpl*         AppendDirectChild    (const char* name, KeyValueType);
    KeyValueImpl*         AppendDirectChild    (KeyValueImpl*);

protected:
    // callbacks for TlvParser
    static bool OnEnterKey(const char* name, void* arg);
    static bool OnLeaveKey(const char* name, void* arg);
    static bool OnValue(const char* name, const char* value, unsigned int value_len, void* arg);

    struct ParseContext
    {
        KeyValueImpl* root_;
        KeyValueImpl* curr_key_;
    };

private:
    KeyValueType          type_;
    std::string           name_;
    std::string           value_;
    unsigned long         children_;
    KeyValueImpl         *first_child_;
    KeyValueImpl         *last_child_;
    KeyValueImpl         *next_sibling_;
    KeyValueImpl         *prev_sibling_;
    KeyValueImpl         *parent_;
};

}}  // namespace fasmio::container

#endif  // CONTAINER_KEY_VALUE_IMPL_H_

