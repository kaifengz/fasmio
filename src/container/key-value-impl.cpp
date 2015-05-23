
#include "./key-value-impl.h"
#include <assert.h>
#include <string.h>

using namespace fasmio::service;

namespace fasmio { namespace container {

KeyValueImpl::KeyValueImpl() :
    type_(KeyValue_Undecided),
    name_(),
    value_(),
    subkeys_(0),
    values_(0),
    first_child_(nullptr),
    last_child_(nullptr),
    next_sibling_(nullptr),
    prev_sibling_(nullptr),
    parent_(nullptr)
{
}

KeyValueImpl::KeyValueImpl(const char* name, KeyValueType type) :
    type_(type),
    name_(name),
    value_(),
    subkeys_(0),
    values_(0),
    first_child_(nullptr),
    last_child_(nullptr),
    next_sibling_(nullptr),
    prev_sibling_(nullptr),
    parent_(nullptr)
{
}

KeyValueImpl::~KeyValueImpl()
{
    Clear();

    // make sure this node has been detached from the key-value-tree
    assert(parent_ == nullptr);
    assert(next_sibling_ == nullptr);
    assert(prev_sibling_ == nullptr);
    assert(first_child_ == nullptr);
    assert(last_child_ == nullptr);
    assert(subkeys_ == 0);
    assert(values_ == 0);
}

bool KeyValueImpl::Serialize(TlvComposer* composer)
{
    if (composer == nullptr)
        return false;

    switch (type_)
    {
    case KeyValue_Value:
        return composer->AddValue(name_.c_str(), reinterpret_cast<const unsigned char*>(value_.c_str()), value_.size());

    case KeyValue_Key:
        if (!composer->BeginKey(name_.c_str()))
            return false;
        for (KeyValueImpl* child = first_child_; child != nullptr; child = child->next_sibling_)
        {
            if (!child->Serialize(composer))
                return false;
        }
        if (!composer->EndKey(name_.c_str()))
            return false;
        return true;

    case KeyValue_Undecided:
    default:
        assert(type_ != KeyValue_Undecided);
        return false;
    }
}

TlvParser::ParseResult KeyValueImpl::UnSerialize(TlvParser* parser)
{
    if (parser == nullptr)
        return TlvParser::R_FAILED;

    if (!parser->PushCallbacks())
        return TlvParser::R_FAILED;

    KeyValueImpl kv;

    ParseContext context;
    context.root_ = &kv;
    context.curr_key_ = nullptr;

    parser->SetEnterKeyCallback(OnEnterKey, &context);
    parser->SetLeaveKeyCallback(OnLeaveKey, &context);
    parser->SetValueCallback(OnValue, &context);
    TlvParser::ParseResult result = parser->Parse();

    if (!parser->RestoreCallbacks())
        return TlvParser::R_FAILED;

    if (result == TlvParser::R_KEY || result == TlvParser::R_VALUE)
        this->Swap(kv);
    return result;
}

void KeyValueImpl::Swap(KeyValueImpl &kv)
{
    std::swap(type_,         kv.type_);
    std::swap(name_,         kv.name_);
    std::swap(value_,        kv.value_);
    std::swap(subkeys_,      kv.subkeys_);
    std::swap(values_,       kv.values_);
    std::swap(first_child_,  kv.first_child_);
    std::swap(last_child_,   kv.last_child_);
    std::swap(next_sibling_, kv.next_sibling_);
    std::swap(prev_sibling_, kv.prev_sibling_);
    std::swap(parent_,       kv.parent_);
}

const char* KeyValueImpl::GetName()
{
    return name_.c_str();
}

bool KeyValueImpl::SetName(const char* name)
{
    if (name == nullptr)
        return false;

    name_ = name;
    return true;
}

IKey* KeyValueImpl::GetParentKey()
{
    return static_cast<IKey*>(parent_);
}

IValue* KeyValueImpl::CloneValue()
{
    return static_cast<IValue*>(CloneKeyValue());
}

const char* KeyValueImpl::GetTextValue()
{
    return value_.c_str();
}

long KeyValueImpl::GetLongValue()
{
    long value = 0;
    TryGetLongValue(&value);
    return value;
}

double KeyValueImpl::GetDoubleValue()
{
    double value = 0.0;
    TryGetDoubleValue(&value);
    return value;
}

bool KeyValueImpl::GetBoolValue()
{
    bool value = false;
    TryGetBoolValue(&value);
    return value;
}

bool KeyValueImpl::TryGetLongValue(long *value_ret)
{
    if (value_ret == nullptr)
        return false;

    const char* svalue = GetTextValue();
    if (svalue == nullptr)
        return false;

    long value = 0;
    unsigned int end = 0;
    if (1 != sscanf(svalue, "%ld%n", &value, &end))
        return false;
    if (end != strlen(svalue))
        return false;

    *value_ret = value;
    return true;
}

bool KeyValueImpl::TryGetDoubleValue(double *value_ret)
{
    if (value_ret == nullptr)
        return false;

    const char* svalue = GetTextValue();
    if (svalue == nullptr)
        return false;

    double value = 0;
    unsigned int end = 0;
    if (1 != sscanf(svalue, "%lf%n", &value, &end))
        return false;
    if (end != strlen(svalue))
        return false;

    *value_ret = value;
    return true;
}

bool KeyValueImpl::TryGetBoolValue(bool *value_ret)
{
    if (value_ret == nullptr)
        return false;

    const char* svalue = GetTextValue();
    if (svalue == nullptr)
        return false;

    if (0 == strcasecmp(svalue, "1") ||
        0 == strcasecmp(svalue, "true") ||
        0 == strcasecmp(svalue, "yes") ||
        0 == strcasecmp(svalue, "on"))
    {
        *value_ret = true;
        return true;
    }

    if (0 == strcasecmp(svalue, "0") ||
        0 == strcasecmp(svalue, "false") ||
        0 == strcasecmp(svalue, "no") ||
        0 == strcasecmp(svalue, "off"))
    {
        *value_ret = false;
        return true;
    }

    return false;
}

bool KeyValueImpl::SetTextValue(const char* value)
{
    assert(type_ == KeyValue_Undecided || type_ == KeyValue_Value);

    if (value == nullptr)
        return false;

    value_ = value;
    if (type_ == KeyValue_Undecided)
    {
        if (parent_ != nullptr)
            ++(parent_->values_);
        type_ = KeyValue_Value;
    }
    return true;
}

bool KeyValueImpl::SetLongValue(long value)
{
    assert(type_ == KeyValue_Undecided || type_ == KeyValue_Value);

    char buff[32];
    snprintf(buff, sizeof(buff), "%ld", value);
    value_ = buff;
    if (type_ == KeyValue_Undecided)
    {
        if (parent_ != nullptr)
            ++(parent_->values_);
        type_ = KeyValue_Value;
    }
    return true;
}

bool KeyValueImpl::SetDoubleValue(double value)
{
    assert(type_ == KeyValue_Undecided || type_ == KeyValue_Value);

    char buff[64];
    snprintf(buff, sizeof(buff), "%lf", value);
    value_ = buff;
    if (type_ == KeyValue_Undecided)
    {
        if (parent_ != nullptr)
            ++(parent_->values_);
        type_ = KeyValue_Value;
    }
    return true;
}

bool KeyValueImpl::SetBoolValue(bool value)
{
    assert(type_ == KeyValue_Undecided || type_ == KeyValue_Value);

    value_ = (value ? "1" : "0");
    if (type_ == KeyValue_Undecided)
    {
        if (parent_ != nullptr)
            ++(parent_->values_);
        type_ = KeyValue_Value;
    }
    return true;
}

IValue* KeyValueImpl::GetNextValue()
{
    return static_cast<IValue*>(GetNextSibling(KeyValue_Value));
}

IValue* KeyValueImpl::GetPrevValue()
{
    return static_cast<IValue*>(GetPrevSibling(KeyValue_Value));
}

IKey* KeyValueImpl::CloneKey()
{
    return static_cast<IKey*>(CloneKeyValue());
}

void KeyValueImpl::Clear()
{
    KeyValueImpl *child = first_child_;
    while (child != nullptr)
    {
        KeyValueImpl *next = child->next_sibling_;

        child->parent_ = nullptr;
        child->next_sibling_ = nullptr;
        child->prev_sibling_ = nullptr;
        delete child;
        child = next;
    }

    this->subkeys_ = 0;
    this->values_ = 0;
    this->first_child_ = nullptr;
    this->last_child_ = nullptr;
}

unsigned long KeyValueImpl::ChildrenCount()
{
    return subkeys_ + values_;
}

unsigned long KeyValueImpl::SubKeyCount()
{
    return subkeys_;
}

unsigned long KeyValueImpl::ValueCount()
{
    return values_;
}

IKey* KeyValueImpl::GetNextKey()
{
    return static_cast<IKey*>(GetNextSibling(KeyValue_Key));
}

IKey* KeyValueImpl::GetPrevKey()
{
    return static_cast<IKey*>(GetPrevSibling(KeyValue_Key));
}

IKey* KeyValueImpl::GetFirstChildKey()
{
    return static_cast<IKey*>(GetFirstChild(KeyValue_Key));
}

IKey* KeyValueImpl::GetLastChildKey()
{
    return static_cast<IKey*>(GetLastChild(KeyValue_Key));
}

IKey* KeyValueImpl::GetKey(const char* name)
{
    return static_cast<IKey*>(GetDeepChild(name, KeyValue_Key, false));
}

IKey* KeyValueImpl::AppendKey(const char* name)
{
    return static_cast<IKey*>(AppendDeepChild(name, KeyValue_Key));
}

IKey* KeyValueImpl::ReleaseKey(const char* name)
{
    return static_cast<IKey*>(ReleaseDeepChild(name, KeyValue_Key));
}

bool KeyValueImpl::DeleteKey(const char* name)
{
    KeyValueImpl* key = ReleaseDeepChild(name, KeyValue_Key);
    if (key == nullptr)
        return false;

    delete key;
    return true;
}

IKey* KeyValueImpl::AppendKey(IKey* key)
{
    KeyValueImpl *kv = dynamic_cast<KeyValueImpl*>(key);
    if (kv == nullptr || (kv->type_ != KeyValue_Undecided && kv->type_ != KeyValue_Key))
        return nullptr;
    kv->type_ = KeyValue_Key;
    return static_cast<IKey*>(AppendDirectChild(kv));
}

IKey* KeyValueImpl::ReleaseKey(IKey* key)
{
    return static_cast<IKey*>(ReleaseDeepChild(dynamic_cast<KeyValueImpl*>(key)));
}

bool KeyValueImpl::DeleteKey(IKey* key)
{
    key = ReleaseDeepChild(dynamic_cast<KeyValueImpl*>(key));
    if (key == nullptr)
        return false;

    delete key;
    return true;
}

IValue* KeyValueImpl::GetValue(const char* name)
{
    return static_cast<IValue*>(GetDeepChild(name, KeyValue_Value, false));
}

IValue* KeyValueImpl::GetFirstValue()
{
    return static_cast<IValue*>(GetFirstChild(KeyValue_Value));
}

IValue* KeyValueImpl::GetLastValue()
{
    return static_cast<IValue*>(GetLastChild(KeyValue_Value));
}

bool KeyValueImpl::DeleteValue(const char* name)
{
    KeyValueImpl* value = ReleaseDeepChild(name, KeyValue_Value);
    if (value == nullptr)
        return false;

    delete value;
    return true;
}

bool KeyValueImpl::DeleteValue(IValue* value)
{
    value = ReleaseDeepChild(dynamic_cast<KeyValueImpl*>(value));
    if (value == nullptr)
        return false;

    delete value;
    return true;
}

const char* KeyValueImpl::GetTextValue(const char* name)
{
    KeyValueImpl* value = GetDeepChild(name, KeyValue_Value, false);
    if (value == nullptr)
        return nullptr;

    return value->GetTextValue();
}

long KeyValueImpl::GetLongValue(const char* name)
{
    KeyValueImpl* value = GetDeepChild(name, KeyValue_Value, false);
    if (value == nullptr)
        return 0;

    return value->GetLongValue();
}

double KeyValueImpl::GetDoubleValue(const char* name)
{
    KeyValueImpl* value = GetDeepChild(name, KeyValue_Value, false);
    if (value == nullptr)
        return 0.0;

    return value->GetDoubleValue();
}

bool KeyValueImpl::GetBoolValue(const char* name)
{
    KeyValueImpl* value = GetDeepChild(name, KeyValue_Value, false);
    if (value == nullptr)
        return false;

    return value->GetBoolValue();
}

bool KeyValueImpl::TryGetLongValue(const char* name, long *value_ret)
{
    KeyValueImpl* value = GetDeepChild(name, KeyValue_Value, false);
    if (value == nullptr)
        return false;

    return value->TryGetLongValue(value_ret);
}

bool KeyValueImpl::TryGetDoubleValue(const char* name, double *value_ret)
{
    KeyValueImpl* value = GetDeepChild(name, KeyValue_Value, false);
    if (value == nullptr)
        return false;

    return value->TryGetDoubleValue(value_ret);
}

bool KeyValueImpl::TryGetBoolValue(const char* name, bool *value_ret)
{
    KeyValueImpl* value = GetDeepChild(name, KeyValue_Value, false);
    if (value == nullptr)
        return false;

    return value->TryGetBoolValue(value_ret);
}

bool KeyValueImpl::SetTextValue(const char* name, const char* value, bool auto_add)
{
    if (value == nullptr)
        return false;

    KeyValueImpl *kv = GetDeepChild(name, KeyValue_Value, auto_add);
    if (kv == nullptr)
        return false;

    return kv->SetTextValue(value);
}

bool KeyValueImpl::SetLongValue(const char* name, long value, bool auto_add)
{
    KeyValueImpl *kv = GetDeepChild(name, KeyValue_Value, auto_add);
    if (kv == nullptr)
        return false;

    return kv->SetLongValue(value);
}

bool KeyValueImpl::SetDoubleValue(const char* name, double value, bool auto_add)
{
    KeyValueImpl *kv = GetDeepChild(name, KeyValue_Value, auto_add);
    if (kv == nullptr)
        return false;

    return kv->SetDoubleValue(value);
}

bool KeyValueImpl::SetBoolValue(const char* name, bool value, bool auto_add)
{
    KeyValueImpl *kv = GetDeepChild(name, KeyValue_Value, auto_add);
    if (kv == nullptr)
        return false;

    return kv->SetBoolValue(value);
}

bool KeyValueImpl::AppendTextValue(const char* name, const char* value)
{
    if (value == nullptr)
        return false;

    KeyValueImpl *kv = AppendDeepChild(name, KeyValue_Value);
    if (kv == nullptr)
        return nullptr;

    return kv->SetTextValue(value);
}

bool KeyValueImpl::AppendLongValue(const char* name, long value)
{
    KeyValueImpl *kv = AppendDeepChild(name, KeyValue_Value);
    if (kv == nullptr)
        return nullptr;

    return kv->SetLongValue(value);
}

bool KeyValueImpl::AppendDoubleValue(const char* name, double value)
{
    KeyValueImpl *kv = AppendDeepChild(name, KeyValue_Value);
    if (kv == nullptr)
        return nullptr;

    return kv->SetDoubleValue(value);
}

bool KeyValueImpl::AppendBoolValue(const char* name, bool value)
{
    KeyValueImpl *kv = AppendDeepChild(name, KeyValue_Value);
    if (kv == nullptr)
        return nullptr;

    return kv->SetBoolValue(value);
}

KeyValueImpl* KeyValueImpl::CloneKeyValue()
{
    KeyValueImpl* cloned = new KeyValueImpl();
    cloned->name_ = this->name_;
    cloned->type_ = this->type_;
    cloned->value_ = this->value_;

    for (KeyValueImpl* child = first_child_; child != nullptr; child = child->next_sibling_)
        cloned->AppendDirectChild(child->CloneKeyValue());

    return cloned;
}

void KeyValueImpl::Dump(int indent)
{
    if (type_ == KeyValue_Key)
    {
        printf("%-*sKey %s\n", indent*2, "", name_.c_str());
        for (KeyValueImpl* child = first_child_; child != nullptr; child = child->next_sibling_)
            child->Dump(indent+1);
    }
    else if (type_ == KeyValue_Value)
    {
        printf("%-*sValue %s = '%s'\n", indent*2, "", name_.c_str(), value_.c_str());
    }
}

KeyValueImpl* KeyValueImpl::GetFirstChild(KeyValueType type)
{
    for (KeyValueImpl* child = first_child_; child != nullptr; child = child->next_sibling_)
    {
        if (child->type_ == type)
            return child;
    }

    return nullptr;
}

KeyValueImpl* KeyValueImpl::GetLastChild(KeyValueType type)
{
    for (KeyValueImpl* child = last_child_; child != nullptr; child = child->prev_sibling_)
    {
        if (child->type_ == type)
            return child;
    }

    return nullptr;
}

KeyValueImpl* KeyValueImpl::GetNextSibling(KeyValueType type)
{
    for (KeyValueImpl* sibling = next_sibling_; sibling != nullptr; sibling = sibling->next_sibling_)
    {
        if (sibling->type_ == type)
            return sibling;
    }

    return nullptr;
}

KeyValueImpl* KeyValueImpl::GetPrevSibling(KeyValueType type)
{
    for (KeyValueImpl* sibling = prev_sibling_; sibling != nullptr; sibling = sibling->prev_sibling_)
    {
        if (sibling->type_ == type)
            return sibling;
    }

    return nullptr;
}

KeyValueImpl* KeyValueImpl::GetDeepChild(const char* name, KeyValueType type, bool auto_add)
{
    if (name == nullptr)
        return nullptr;

    assert(type == KeyValue_Key || type == KeyValue_Value);

    KeyValueImpl *kv = nullptr;
    if (nullptr == strchr(name, '/'))
    {
        if (nullptr == (kv = GetDirectChild(name, type)) && auto_add)
            kv = AppendDirectChild(name, type);
    }
    else
    {
        char* name_copy = strdup(name);
        kv = AppendGetDeepChild(name_copy, type, false, auto_add);
        free(name_copy);
    }

    return kv;
}

KeyValueImpl* KeyValueImpl::AppendDeepChild(const char* name, KeyValueType type)
{
    if (name == nullptr)
        return nullptr;

    assert(type == KeyValue_Key || type == KeyValue_Value);

    KeyValueImpl *kv = nullptr;
    if (nullptr == strchr(name, '/'))
        kv = AppendDirectChild(name, type);
    else
    {
        char* name_copy = strdup(name);
        kv = AppendGetDeepChild(name_copy, type, true);
        free(name_copy);
    }

    return kv;
}

KeyValueImpl* KeyValueImpl::ReleaseDeepChild(const char* name, KeyValueType type)
{
    KeyValueImpl* kv = GetDeepChild(name, type, false);
    if (kv == nullptr)
        return nullptr;

    return ReleaseDeepChild(kv);
}

KeyValueImpl* KeyValueImpl::ReleaseDeepChild(KeyValueImpl* kv)
{
    if (kv == nullptr || kv->parent_ == nullptr)
        return nullptr;

    if (kv->prev_sibling_ == nullptr)
        kv->parent_->first_child_ = kv->next_sibling_;
    else
        kv->prev_sibling_->next_sibling_ = kv->next_sibling_;

    if (kv->next_sibling_ == nullptr)
        kv->parent_->last_child_ = kv->prev_sibling_;
    else
        kv->next_sibling_->prev_sibling_ = kv->prev_sibling_;

    if (kv->type_ == KeyValue_Key)
        --(kv->parent_->subkeys_);
    else if (kv->type_ == KeyValue_Value)
        --(kv->parent_->values_);

    kv->parent_ = nullptr;
    kv->next_sibling_ = nullptr;
    kv->prev_sibling_ = nullptr;
    return kv;
}

KeyValueImpl* KeyValueImpl::AppendGetDeepChild(char* name, KeyValueType type, bool append, bool auto_add)
{
    char* slash = strchr(name, '/');
    if (slash == nullptr)
    {
        if (append)
            return AppendDirectChild(name, type);
        else
        {
            KeyValueImpl *kv = GetDirectChild(name, type);
            if (kv == nullptr && auto_add)
                kv = AppendDirectChild(name, type);
            return kv;
        }
    }
    else
    {
        const char* subkey_name = name;
        char* remain_name = slash + 1;
        *slash = '\0';

        KeyValueImpl* subkey = GetDirectChild(subkey_name, KeyValue_Key);
        if (subkey == nullptr)
        {
            if (append || auto_add)
                subkey = AppendDirectChild(subkey_name, KeyValue_Key);
            if (subkey == nullptr)
                return nullptr;
        }

        return subkey->AppendGetDeepChild(remain_name, type, append, auto_add);
    }
}

KeyValueImpl* KeyValueImpl::GetDirectChild(const char* name, KeyValueType type)
{
    for (KeyValueImpl* child = first_child_; child != nullptr; child = child->next_sibling_)
    {
        if (child->type_ == type && 0 == strcasecmp(child->name_.c_str(), name))
            return child;
    }

    return nullptr;
}

KeyValueImpl* KeyValueImpl::AppendDirectChild(const char* name, KeyValueType type)
{
    KeyValueImpl *kv = new KeyValueImpl();
    if (kv == nullptr)
        return nullptr;

    kv->name_ = name;
    kv->type_ = type;
    return AppendDirectChild(kv);
}

KeyValueImpl* KeyValueImpl::AppendDirectChild(KeyValueImpl* kv)
{
    if (kv->parent_ != nullptr || kv->next_sibling_ != nullptr || kv->prev_sibling_ != nullptr)
        return nullptr;

    assert(type_ == KeyValue_Undecided || type_ == KeyValue_Key);
    if (type_ == KeyValue_Undecided)
    {
        if (parent_ != nullptr)
            ++(parent_->subkeys_);
        type_ = KeyValue_Key;
    }

    kv->parent_ = this;
    kv->prev_sibling_ = this->last_child_;
    kv->next_sibling_ = nullptr;
    if (this->last_child_ != nullptr)
    {
        assert(this->first_child_ != nullptr);
        this->last_child_->next_sibling_ = kv;
    }
    else
    {
        assert(this->first_child_ == nullptr);
        this->first_child_ = kv;
    }
    this->last_child_ = kv;

    if (kv->type_ == KeyValue_Key)
        ++(this->subkeys_);
    else if (kv->type_ == KeyValue_Value)
        ++(this->values_);
    return kv;
}

bool KeyValueImpl::OnEnterKey(const char* name, void* arg)
{
    ParseContext *context = reinterpret_cast<ParseContext*>(arg);

    if (context->curr_key_ == nullptr)
    {
        context->curr_key_ = context->root_;
        context->curr_key_->type_ = KeyValue_Key;
        context->curr_key_->name_ = name;
    }
    else
    {
        KeyValueImpl *subkey = context->curr_key_->AppendDirectChild(name, KeyValue_Key);
        if (subkey == nullptr)
            return false;

        context->curr_key_ = subkey;
    }

    return true;
}

bool KeyValueImpl::OnLeaveKey(const char* name, void* arg)
{
    ParseContext *context = reinterpret_cast<ParseContext*>(arg);

    assert(0 == strcmp(context->curr_key_->name_.c_str(), name));
    assert(context->curr_key_->type_ == KeyValue_Key);

    context->curr_key_ = context->curr_key_->parent_;
    return true;
}

bool KeyValueImpl::OnValue(const char* name, const char* svalue, unsigned int svalue_len, void* arg)
{
    ParseContext *context = reinterpret_cast<ParseContext*>(arg);

    if (context->curr_key_ == nullptr)
    {
        context->curr_key_ = context->root_;
        context->curr_key_->type_ = KeyValue_Value;
        context->curr_key_->name_ = name;
        context->curr_key_->value_.assign(svalue, svalue_len);
    }
    else
    {
        KeyValueImpl *value = context->curr_key_->AppendDirectChild(name, KeyValue_Value);
        if (value == nullptr)
            return false;

        value->value_.assign(svalue, svalue_len);
    }

    return true;
}

}}  // namespace fasmio::container

