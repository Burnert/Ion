#pragma once

#include "Core/CoreApi.h"
#include "Core/CoreTypes.h"
#include "Core/Logging/Logger.h"

namespace Ion
{
	namespace Serialisation
	{
		template<typename ClassT, typename FieldT>
		void DeserialiseField(ClassT* object, FieldT ClassT::*field, byte* serialisedData)
		{
			ullong fieldOffset = (ullong)&(((ClassT*)0)->*field);
			FieldT* data = (FieldT*)(serialisedData + fieldOffset);
			object->*field = *data;
		}

		template<typename ClassT, typename FieldT>
		void SerialiseField(ClassT* object, FieldT ClassT::*field, byte* serialisedData)
		{
			ullong fieldOffset = (ullong)&(((ClassT*)0)->*field);
			FieldT& fieldRef = object->*field;
			*(FieldT*)(serialisedData + fieldOffset) = fieldRef;
		}
	}

	class ION_API ISerialisable
	{
	public:
		/* Transforms an object to a series of bytes */
		virtual void Serialise(byte* serialisedData) = 0;
		/* Loads serialised data to the object it is called on */
		virtual void Deserialise(byte* serialisedData) = 0;
	};

	struct SerialTest : public ISerialisable
	{
		// @TODO: Fix the size problem (unnecessary vftable pointer in the serial)

		float a;
		int b;
		long c;
		uint d;

		virtual void Serialise(byte* serialisedData) override
		{
			Serialisation::SerialiseField(this, &SerialTest::a, serialisedData);
			Serialisation::SerialiseField(this, &SerialTest::b, serialisedData);
			Serialisation::SerialiseField(this, &SerialTest::c, serialisedData);
			Serialisation::SerialiseField(this, &SerialTest::d, serialisedData);
		}

		virtual void Deserialise(byte* serialisedData) override
		{
			Serialisation::DeserialiseField(this, &SerialTest::a, serialisedData);
			Serialisation::DeserialiseField(this, &SerialTest::b, serialisedData);
			Serialisation::DeserialiseField(this, &SerialTest::c, serialisedData);
			Serialisation::DeserialiseField(this, &SerialTest::d, serialisedData);
		}
	};

	void SerialisationTest()
	{
		SerialTest t;
		t.a = 93.2f;
		t.b = -51;
		t.c = -55411;
		t.d = 666601;

		byte tSerial[sizeof(SerialTest)];
		((ISerialisable*)&t)->Serialise(tSerial);

		SerialTest t2;
		t2.Deserialise(tSerial);

		ASSERT(t2.a == t.a);
		ASSERT(t2.b == t.b);
		ASSERT(t2.c == t.c);
		ASSERT(t2.d == t.d);

		LOG_INFO("SerialTest: {0}, {1}, {2}, {3}", t2.a, t2.b, t2.c, t2.d);
	}
}
