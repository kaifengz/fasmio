
#include "gtest/gtest.h"

#include <memory>
#include "container/key-value-impl.h"
#include "container/stream-impl.h"

using namespace fasmio::container;
using namespace fasmio::service;

namespace {

    TEST(KeyValueImpl, Name)
    {
        KeyValueImpl kv;

        EXPECT_EQ(std::string(""), kv.GetName());
        EXPECT_TRUE(kv.SetName("my-name"));
        EXPECT_EQ(std::string("my-name"), kv.GetName());
    }

    TEST(KeyValueImpl, Parent)
    {
        KeyValueImpl kv;

        EXPECT_TRUE(nullptr != kv.AppendKey("first/second/third"));
        EXPECT_EQ(&kv, kv.GetKey("first")->GetParentKey());
        EXPECT_EQ(kv.GetKey("first"), kv.GetKey("first/second")->GetParentKey());
        EXPECT_EQ(kv.GetKey("first/second"), kv.GetKey("first/second/third")->GetParentKey());
    }

    TEST(KeyValueImpl, GetValue1)
    {
        KeyValueImpl kv;

        EXPECT_TRUE(kv.AppendTextValue("text", "value"));
        EXPECT_TRUE(kv.AppendLongValue("long", 123));
        EXPECT_TRUE(kv.AppendDoubleValue("double", 234.5));
        EXPECT_TRUE(kv.AppendBoolValue("bool", true));

        EXPECT_EQ(std::string("value"), kv.GetTextValue("text"));
        EXPECT_EQ(std::string("123"), kv.GetTextValue("long"));
        EXPECT_EQ(std::string("234.500000"), kv.GetTextValue("double"));
        EXPECT_EQ(std::string("1"), kv.GetTextValue("bool"));

        EXPECT_EQ(0, kv.GetLongValue("text"));
        EXPECT_EQ(123, kv.GetLongValue("long"));
        EXPECT_EQ(0, kv.GetLongValue("double"));
        EXPECT_EQ(1, kv.GetLongValue("bool"));

        EXPECT_EQ(0.0, kv.GetDoubleValue("text"));
        EXPECT_EQ(123.0, kv.GetDoubleValue("long"));
        EXPECT_EQ(234.5, kv.GetDoubleValue("double"));
        EXPECT_EQ(1.0, kv.GetDoubleValue("bool"));

        EXPECT_TRUE(!kv.GetBoolValue("text"));
        EXPECT_TRUE(!kv.GetBoolValue("long"));
        EXPECT_TRUE(!kv.GetBoolValue("double"));
        EXPECT_TRUE(kv.GetBoolValue("bool"));
    }

    TEST(KeyValueImpl, GetValue2)
    {
        KeyValueImpl kv;

        EXPECT_TRUE(kv.AppendTextValue("text", "value"));
        EXPECT_TRUE(kv.AppendLongValue("long", 123));
        EXPECT_TRUE(kv.AppendDoubleValue("double", 234.5));
        EXPECT_TRUE(kv.AppendBoolValue("bool", true));

        IValue* tvalue = kv.GetValue("text");
        IValue* lvalue = kv.GetValue("long");
        IValue* dvalue = kv.GetValue("double");
        IValue* bvalue = kv.GetValue("bool");

        EXPECT_TRUE(nullptr != tvalue);
        EXPECT_TRUE(nullptr != lvalue);
        EXPECT_TRUE(nullptr != dvalue);
        EXPECT_TRUE(nullptr != bvalue);

        EXPECT_EQ(std::string("value"), tvalue->GetTextValue());
        EXPECT_EQ(std::string("123"), lvalue->GetTextValue());
        EXPECT_EQ(std::string("234.500000"), dvalue->GetTextValue());
        EXPECT_EQ(std::string("1"), bvalue->GetTextValue());

        EXPECT_EQ(0, tvalue->GetLongValue());
        EXPECT_EQ(123, lvalue->GetLongValue());
        EXPECT_EQ(0, dvalue->GetLongValue());
        EXPECT_EQ(1, bvalue->GetLongValue());

        EXPECT_EQ(0.0, tvalue->GetDoubleValue());
        EXPECT_EQ(123.0, lvalue->GetDoubleValue());
        EXPECT_EQ(234.5, dvalue->GetDoubleValue());
        EXPECT_EQ(1.0, bvalue->GetDoubleValue());

        EXPECT_TRUE(!tvalue->GetBoolValue());
        EXPECT_TRUE(!lvalue->GetBoolValue());
        EXPECT_TRUE(!dvalue->GetBoolValue());
        EXPECT_TRUE(bvalue->GetBoolValue());
    }

    TEST(KeyValueImpl, TryGetValue1)
    {
        KeyValueImpl kv;

        EXPECT_TRUE(kv.AppendTextValue("text", "value"));
        EXPECT_TRUE(kv.AppendLongValue("long", 123));
        EXPECT_TRUE(kv.AppendDoubleValue("double", 234.5));
        EXPECT_TRUE(kv.AppendBoolValue("bool", true));

        long lv = 0;
        double dv = 0.0;
        bool bv = false;

        EXPECT_TRUE(!kv.TryGetLongValue("text", &lv));
        EXPECT_TRUE(kv.TryGetLongValue("long", &lv));
        EXPECT_EQ(123, lv);
        EXPECT_TRUE(!kv.TryGetLongValue("double", &lv));
        EXPECT_TRUE(kv.TryGetLongValue("bool", &lv));
        EXPECT_EQ(1, lv);

        EXPECT_TRUE(!kv.TryGetDoubleValue("text", &dv));
        EXPECT_TRUE(kv.TryGetDoubleValue("long", &dv));
        EXPECT_EQ(123.0, dv);
        EXPECT_TRUE(kv.TryGetDoubleValue("double", &dv));
        EXPECT_EQ(234.5, dv);
        EXPECT_TRUE(kv.TryGetDoubleValue("bool", &dv));
        EXPECT_EQ(1.0, dv);

        EXPECT_TRUE(!kv.TryGetBoolValue("text", &bv));
        EXPECT_TRUE(!kv.TryGetBoolValue("long", &bv));
        EXPECT_TRUE(!kv.TryGetBoolValue("double", &bv));
        EXPECT_TRUE(kv.TryGetBoolValue("bool", &bv));
        EXPECT_TRUE(bv);
    }

    TEST(KeyValueImpl, TryGetValue2)
    {
        KeyValueImpl kv;

        EXPECT_TRUE(kv.AppendTextValue("text", "value"));
        EXPECT_TRUE(kv.AppendLongValue("long", 123));
        EXPECT_TRUE(kv.AppendDoubleValue("double", 234.5));
        EXPECT_TRUE(kv.AppendBoolValue("bool", true));

        IValue* tvalue = kv.GetValue("text");
        IValue* lvalue = kv.GetValue("long");
        IValue* dvalue = kv.GetValue("double");
        IValue* bvalue = kv.GetValue("bool");

        EXPECT_TRUE(nullptr != tvalue);
        EXPECT_TRUE(nullptr != lvalue);
        EXPECT_TRUE(nullptr != dvalue);
        EXPECT_TRUE(nullptr != bvalue);

        long lv = 0;
        double dv = 0.0;
        bool bv = false;

        EXPECT_TRUE(!tvalue->TryGetLongValue(&lv));
        EXPECT_TRUE(lvalue->TryGetLongValue(&lv));
        EXPECT_EQ(123, lv);
        EXPECT_TRUE(!dvalue->TryGetLongValue(&lv));
        EXPECT_TRUE(bvalue->TryGetLongValue(&lv));
        EXPECT_EQ(1, lv);

        EXPECT_TRUE(!tvalue->TryGetDoubleValue(&dv));
        EXPECT_TRUE(lvalue->TryGetDoubleValue(&dv));
        EXPECT_EQ(123.0, dv);
        EXPECT_TRUE(dvalue->TryGetDoubleValue(&dv));
        EXPECT_EQ(234.5, dv);
        EXPECT_TRUE(bvalue->TryGetDoubleValue(&dv));
        EXPECT_EQ(1.0, dv);

        EXPECT_TRUE(!tvalue->TryGetBoolValue(&bv));
        EXPECT_TRUE(!lvalue->TryGetBoolValue(&bv));
        EXPECT_TRUE(!dvalue->TryGetBoolValue(&bv));
        EXPECT_TRUE(bvalue->TryGetBoolValue(&bv));
        EXPECT_TRUE(bv);
    }

    TEST(KeyValueImpl, SetValue)
    {
        KeyValueImpl kv;

        EXPECT_TRUE(nullptr == kv.GetTextValue("text"));
        kv.SetTextValue("text", "value", false);
        EXPECT_TRUE(nullptr == kv.GetTextValue("text"));
        kv.SetTextValue("text", "value", true);
        EXPECT_EQ(std::string("value"), kv.GetTextValue("text"));

        EXPECT_EQ(0, kv.GetLongValue("long"));
        kv.SetLongValue("long", 123, false);
        EXPECT_EQ(0, kv.GetLongValue("long"));
        kv.SetLongValue("long", 123, true);
        EXPECT_EQ(123, kv.GetLongValue("long"));

        EXPECT_EQ(0.0, kv.GetDoubleValue("double"));
        kv.SetDoubleValue("double", 234.5, false);
        EXPECT_EQ(0.0, kv.GetDoubleValue("double"));
        kv.SetDoubleValue("double", 234.5, true);
        EXPECT_EQ(234.5, kv.GetDoubleValue("double"));

        EXPECT_TRUE(!kv.GetBoolValue("bool"));
        kv.SetBoolValue("bool", true, false);
        EXPECT_TRUE(!kv.GetBoolValue("bool"));
        kv.SetBoolValue("bool", true, true);
        EXPECT_TRUE(kv.GetBoolValue("bool"));
    }

    TEST(KeyValueImpl, DeleteValue)
    {
        KeyValueImpl kv;

        EXPECT_TRUE(!kv.DeleteValue("text"));
        EXPECT_TRUE(kv.AppendTextValue("text", "value"));
        EXPECT_TRUE(kv.DeleteValue("text"));
        EXPECT_TRUE(!kv.DeleteValue("text"));

        EXPECT_TRUE(!kv.DeleteValue(static_cast<const char*>(nullptr)));
        EXPECT_TRUE(!kv.DeleteValue(static_cast<IValue*>(nullptr)));

        EXPECT_TRUE(kv.AppendTextValue("text", "value"));
        IValue* value = kv.GetValue("text");
        EXPECT_TRUE(nullptr != value);
        EXPECT_TRUE(kv.DeleteValue(value));
    }

    TEST(KeyValueImpl, EnumerateValue)
    {
        KeyValueImpl kv;

        EXPECT_TRUE(kv.AppendTextValue("value1", "1"));
        EXPECT_TRUE(nullptr != kv.AppendKey("key1/subkey1.1"));
        EXPECT_TRUE(kv.AppendTextValue("value2", "2"));
        EXPECT_TRUE(kv.AppendTextValue("value2", "2 again"));
        EXPECT_TRUE(nullptr != kv.AppendKey("key2/subkey1.1"));
        EXPECT_TRUE(kv.AppendTextValue("value3", "3"));

        IValue* value = nullptr;

        EXPECT_TRUE(nullptr != (value = kv.GetFirstValue()));
        EXPECT_EQ(std::string("value1"), value->GetName());
        EXPECT_EQ(std::string("1"), value->GetTextValue());
        EXPECT_TRUE(nullptr != (value = value->GetNextValue()));
        EXPECT_EQ(std::string("value2"), value->GetName());
        EXPECT_EQ(std::string("2"), value->GetTextValue());
        EXPECT_TRUE(nullptr != (value = value->GetNextValue()));
        EXPECT_EQ(std::string("value2"), value->GetName());
        EXPECT_EQ(std::string("2 again"), value->GetTextValue());
        EXPECT_TRUE(nullptr != (value = value->GetNextValue()));
        EXPECT_EQ(std::string("value3"), value->GetName());
        EXPECT_EQ(std::string("3"), value->GetTextValue());
        EXPECT_TRUE(nullptr == (value = value->GetNextValue()));

        EXPECT_TRUE(nullptr != (value = kv.GetLastValue()));
        EXPECT_EQ(std::string("value3"), value->GetName());
        EXPECT_EQ(std::string("3"), value->GetTextValue());
        EXPECT_TRUE(nullptr != (value = value->GetPrevValue()));
        EXPECT_EQ(std::string("value2"), value->GetName());
        EXPECT_EQ(std::string("2 again"), value->GetTextValue());
        EXPECT_TRUE(nullptr != (value = value->GetPrevValue()));
        EXPECT_EQ(std::string("value2"), value->GetName());
        EXPECT_EQ(std::string("2"), value->GetTextValue());
        EXPECT_TRUE(nullptr != (value = value->GetPrevValue()));
        EXPECT_EQ(std::string("value1"), value->GetName());
        EXPECT_EQ(std::string("1"), value->GetTextValue());
        EXPECT_TRUE(nullptr == (value = value->GetPrevValue()));
    }

    TEST(KeyValueImpl, EnumerateKey)
    {
        KeyValueImpl kv;

        EXPECT_TRUE(kv.AppendTextValue("value1", "1"));
        EXPECT_TRUE(nullptr != kv.AppendKey("key1/subkey1.1"));
        EXPECT_TRUE(kv.AppendTextValue("value2", "2"));
        EXPECT_TRUE(kv.AppendTextValue("value2", "2 again"));
        EXPECT_TRUE(nullptr != kv.AppendKey("key2/subkey1.1"));
        EXPECT_TRUE(kv.AppendTextValue("value3", "3"));

        IKey* key = nullptr;

        EXPECT_TRUE(nullptr != (key = kv.GetFirstChildKey()));
        EXPECT_EQ(std::string("key1"), key->GetName());
        EXPECT_TRUE(nullptr != (key = key->GetNextKey()));
        EXPECT_EQ(std::string("key2"), key->GetName());
        EXPECT_TRUE(nullptr == (key = key->GetNextKey()));

        EXPECT_TRUE(nullptr != (key = kv.GetLastChildKey()));
        EXPECT_EQ(std::string("key2"), key->GetName());
        EXPECT_TRUE(nullptr != (key = key->GetPrevKey()));
        EXPECT_EQ(std::string("key1"), key->GetName());
        EXPECT_TRUE(nullptr == (key = key->GetPrevKey()));
    }

    TEST(KeyValueImpl, DeepAccess)
    {
        KeyValueImpl kv;

        EXPECT_TRUE(!kv.SetTextValue("name1/name1.1/name1.1.1", "failed", false));
        EXPECT_TRUE(nullptr == kv.GetTextValue("name1/name1.1/name1.1.1"));

        EXPECT_TRUE(kv.SetTextValue("name1/name1.1/name1.1.1", "1.1.1", true));
        EXPECT_TRUE(kv.SetTextValue("name1/name1.2/name1.2.1/name1.2.1.1", "1.2.1.1", true));
        EXPECT_TRUE(kv.SetTextValue("name1/name1.1/name1.1.1", "1.1.1-new", true));
        EXPECT_TRUE(kv.SetTextValue("name1/name1.2/name1.2.1/name1.2.1.2", "1.2.1.2", true));
        EXPECT_TRUE(kv.AppendTextValue("name1/name1.1/name1.1.1", "1.1.1-last"));

        IKey* key11 = kv.GetKey("name1/name1.1");
        EXPECT_TRUE(nullptr != key11);
        EXPECT_EQ(std::string("1.1.1-new"), key11->GetTextValue("name1.1.1"));
        EXPECT_EQ(std::string("1.1.1-last"), key11->GetLastValue()->GetTextValue());

        EXPECT_EQ(std::string("1.2.1.1"), kv.GetTextValue("name1/name1.2/name1.2.1/name1.2.1.1"));
        EXPECT_EQ(std::string("1.2.1.2"), kv.GetTextValue("name1/name1.2/name1.2.1/name1.2.1.2"));
    }

    TEST(KeyValueImpl, ReleaseAndAppend)
    {
        KeyValueImpl kv;

        EXPECT_TRUE(kv.AppendTextValue("first/second/third/name", "value"));
        EXPECT_TRUE(nullptr == kv.ReleaseKey("nothing"));

        IKey *subkey = nullptr;
        EXPECT_TRUE(nullptr != kv.GetKey("first/second"));
        EXPECT_TRUE(nullptr != (subkey = kv.ReleaseKey("first/second")));
        EXPECT_TRUE(nullptr == kv.GetKey("first/second"));
        EXPECT_EQ(std::string("second"), subkey->GetName());
        EXPECT_EQ(std::string("value"), subkey->GetTextValue("third/name"));

        EXPECT_TRUE(subkey == kv.AppendKey(subkey));
        EXPECT_TRUE(subkey == kv.GetKey("second"));
        EXPECT_EQ(std::string("value"), kv.GetTextValue("second/third/name"));

        EXPECT_TRUE(!kv.DeleteValue("nothing"));
        EXPECT_TRUE(kv.DeleteValue("second/third/name"));
        EXPECT_TRUE(nullptr == kv.GetTextValue("second/third/name"));
        EXPECT_TRUE(nullptr != kv.GetKey("second/third"));
        EXPECT_TRUE(kv.DeleteKey("second/third"));
        EXPECT_TRUE(nullptr == kv.GetKey("second/third"));
    }

    TEST(KeyValueImpl, Clone)
    {
        KeyValueImpl kv;

        EXPECT_TRUE(kv.AppendTextValue("first/second/name", "value1"));
        EXPECT_TRUE(kv.AppendTextValue("first/second", "value2"));
        EXPECT_TRUE(kv.AppendTextValue("first", "value3"));

        std::unique_ptr<IKey> cloned((static_cast<IKey*>(&kv))->Clone());
        EXPECT_TRUE(nullptr != cloned.get());

        EXPECT_EQ(std::string("value1"), cloned->GetTextValue("first/second/name"));
        EXPECT_EQ(std::string("value2"), cloned->GetTextValue("first/second"));
        EXPECT_EQ(std::string("value3"), cloned->GetTextValue("first"));

        EXPECT_TRUE(cloned->SetTextValue("first", "new-value", false));
        EXPECT_EQ(std::string("new-value"), cloned->GetTextValue("first"));
        EXPECT_EQ(std::string("value3"), kv.GetTextValue("first"));
    }

    TEST(KeyValueImpl, Serialize_Key)
    {
        KeyValueImpl kv;

        EXPECT_TRUE(kv.SetName("root"));
        EXPECT_TRUE(kv.AppendTextValue("first/second/name", "value1"));
        EXPECT_TRUE(kv.AppendTextValue("first/second", "value2"));
        EXPECT_TRUE(kv.AppendTextValue("first", "value3"));

        StreamImpl stream;
        TlvComposer composer(&stream);
        EXPECT_TRUE(kv.Serialize(&composer));
        EXPECT_TRUE(composer.CheckIntegrity());

        KeyValueImpl kv2;
        TlvParser parser(&stream);
        EXPECT_TRUE(kv2.UnSerialize(&parser));

        EXPECT_EQ(std::string("root"), kv2.GetName());
        EXPECT_EQ(std::string("value1"), kv2.GetTextValue("first/second/name"));
        EXPECT_EQ(std::string("value2"), kv2.GetTextValue("first/second"));
        EXPECT_EQ(std::string("value3"), kv2.GetTextValue("first"));
    }

    TEST(KeyValueImpl, Serialize_Value)
    {
        KeyValueImpl kv;

        EXPECT_TRUE(kv.SetName("name"));
        EXPECT_TRUE(kv.SetTextValue("value"));

        StreamImpl stream;
        TlvComposer composer(&stream);
        EXPECT_TRUE(kv.Serialize(&composer));
        EXPECT_TRUE(composer.CheckIntegrity());

        KeyValueImpl kv2;
        TlvParser parser(&stream);
        EXPECT_TRUE(kv2.UnSerialize(&parser));

        EXPECT_EQ(std::string("name"), kv2.GetName());
        EXPECT_EQ(std::string("value"), kv2.GetTextValue());
    }

    TEST(KeyValueImpl, Serialize_Multiple)
    {
        KeyValueImpl kv1;
        EXPECT_TRUE(kv1.SetName("root"));
        EXPECT_TRUE(kv1.AppendTextValue("first/second/name", "value1"));
        EXPECT_TRUE(kv1.AppendTextValue("first/second", "value2"));
        EXPECT_TRUE(kv1.AppendTextValue("first", "value3"));

        KeyValueImpl kv2;
        EXPECT_TRUE(kv2.SetName("name"));
        EXPECT_TRUE(kv2.SetTextValue("value"));

        StreamImpl stream;
        TlvComposer composer(&stream);
        EXPECT_TRUE(kv1.Serialize(&composer));
        EXPECT_TRUE(composer.CheckIntegrity());
        EXPECT_TRUE(kv2.Serialize(&composer));
        EXPECT_TRUE(composer.CheckIntegrity());

        TlvParser parser(&stream);

        KeyValueImpl kv1c;
        EXPECT_TRUE(kv1c.UnSerialize(&parser));
        EXPECT_EQ(std::string("root"), kv1c.GetName());
        EXPECT_EQ(std::string("value1"), kv1c.GetTextValue("first/second/name"));
        EXPECT_EQ(std::string("value2"), kv1c.GetTextValue("first/second"));
        EXPECT_EQ(std::string("value3"), kv1c.GetTextValue("first"));

        KeyValueImpl kv2c;
        EXPECT_TRUE(kv2c.UnSerialize(&parser));
        EXPECT_EQ(std::string("name"), kv2c.GetName());
        EXPECT_EQ(std::string("value"), kv2c.GetTextValue());

        KeyValueImpl kv3n;
        EXPECT_TRUE(!kv3n.UnSerialize(&parser));
    }

    TEST(KeyValueImpl, UnSerialize_Empty)
    {
        KeyValueImpl kv;

        StreamImpl stream;
        TlvParser parser(&stream);
        EXPECT_TRUE(!kv.UnSerialize(&parser));
    }

    TEST(KeyValueImpl, Count)
    {
        KeyValueImpl kv;
        EXPECT_EQ(0, kv.ChildrenCount());
        EXPECT_EQ(0, kv.SubKeyCount());
        EXPECT_EQ(0, kv.ValueCount());

        kv.AppendKey(new KeyValueImpl("what"));
        EXPECT_EQ(1, kv.ChildrenCount());
        EXPECT_EQ(1, kv.SubKeyCount());
        EXPECT_EQ(0, kv.ValueCount());

        kv.AppendTextValue("why", "why not");
        EXPECT_EQ(2, kv.ChildrenCount());
        EXPECT_EQ(1, kv.SubKeyCount());
        EXPECT_EQ(1, kv.ValueCount());
    }
}

