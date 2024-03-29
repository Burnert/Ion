#pragma once

#include "Core/Base.h"
#include "Core/Templates/Templates.h"

namespace Ion
{
	namespace Serialisation
	{
		/* Size of the length value for serialisation (8 bytes) */
		constexpr uint8 LengthSize = sizeof(uint64);
		constexpr uint64 MaxFieldSize = ULLONG_MAX;

		enum class EState
		{
			NULLSTATE,
			INIT,
			SERIALISING,
			DESERIALISING,
			WRITING,
			READING,
			FINALISED
		};

		enum class EType
		{
			UNDEFINED,
			WRITE,
			READ
		};

		class Serial;

		class ION_API ISerialisable
		{
		public:
			/* Transforms an object to a series of bytes */
			virtual void Serialise(Serial* serial) = 0;
			/* Loads serialised data to the object it is called on */
			virtual void Deserialise(Serial* serial) = 0;
		};

		class ION_API Serial
		{
			template<typename SerialisableT, TEnableIfT<TIsBaseOfV<ISerialisable, SerialisableT>, bool>>
			friend class TClassSerialiser;

			template<typename T> friend void SerialiseStruct(T* structPtr, Serial* serial);
			template<typename T> friend void DeserialiseStruct(T* structPtr, Serial* serial);

		public:
			Serial() :
				m_Initialised(false),
				m_Bytes(nullptr),
				m_Size(0)
			{ }

			virtual ~Serial()
			{
				if (m_Bytes)
					delete[] m_Bytes;
			}

			uint64 GetSize() const
			{
				return m_Size;
			}

			/* Returns raw serialised bytes (read-only). */
			const uint8* const GetImmutableBytes() const
			{
				return m_Bytes;
			}

		private:
			/* Sets pointer to bytes of this object to the one specified.
			   Changes the ownership of the bytes to this Serial. */
			void PushBytes(uint8* bytes, uint64 count)
			{
				if (!m_Initialised)
				{
					if (m_Bytes)
						delete[] m_Bytes;

					m_Bytes = bytes;
					m_Size = count;
					m_Initialised = true;
				}
			}

			/* Returns bytes and uninitialises the pointer in this object.
			   Removes the ownership of the bytes from this Serial. */
			uint8* PullBytes(uint64* outCount)
			{
				if (m_Initialised)
				{
					if (!m_Bytes)
						return nullptr;

					*outCount = m_Size;
					uint8* temp = m_Bytes;
					m_Bytes = nullptr;
					m_Size = 0;
					m_Initialised = false;
					return temp;
				}
				outCount = 0;
				return nullptr;
			}

			/* Creates a buffer of this Serial's bytes preceded by the size of it.
			   The buffer must then be deleted using delete[] as this Serial does not own it. */
			uint8* CreateSignedBuffer()
			{
				if (m_Initialised && m_Size > 0)
				{
					uint8* buffer = new uint8[m_Size + LengthSize];
					memcpy_s(buffer, LengthSize, &m_Size, LengthSize);
					memcpy_s(buffer + LengthSize, m_Size, m_Bytes, m_Size);
					return buffer;
				}
				return nullptr;
			}

		private:
			bool m_Initialised;
			uint64 m_Size;
			uint8* m_Bytes;
		};

		// @TODO: Write docs ASAP...

		template<typename SerialisableT, TEnableIfT<TIsBaseOfV<ISerialisable, SerialisableT>, bool> = true>
		class TClassSerialiser
		{
		public:
			TClassSerialiser(SerialisableT* serialisable, EType type) :
				m_Type(type),
				m_SerialisablePtr(serialisable),
				m_BytesCounter(0),
				m_MaxCount(0),
				m_State(EState::INIT)
			{
				if (type == EType::WRITE)
				{
					// @TODO: Make the buffer resizable instead of this nonsense 64 here.
					m_Bytes = new uint8[sizeof(SerialisableT) * 64];
				}
				else
				{
					m_Bytes = nullptr;
				}
			}

			TClassSerialiser() :
				m_Type(EType::UNDEFINED),
				m_SerialisablePtr(nullptr),
				m_BytesCounter(0),
				m_MaxCount(0),
				m_Bytes(nullptr),
				m_State(EState::NULLSTATE)
			{ }
			
			virtual ~TClassSerialiser()
			{
				if (m_Bytes)
				{
					delete[] m_Bytes;
				}
				if (m_State != EState::FINALISED)
				{
					CoreLogger.Warn("Serialiser was destroyed before Finalise() was called! This might be a bug.");
				}
			}

			void Init(SerialisableT* serialisable, EType type)
			{
				if (!(m_State == EState::NULLSTATE || m_State == EState::FINALISED))
				{
					CoreLogger.Critical("Cannot initialise serialiser in this state! {0}", m_State);
					ionassert(false);
					return;
				}
				m_Type = type;
				m_SerialisablePtr = serialisable;
				m_BytesCounter = 0;
				m_MaxCount = 0;
				m_Bytes = new uint8[sizeof(SerialisableT)];
				m_State = EState::INIT;
			}

			template<typename FieldT, typename... RestT>
			void Serialise(FieldT SerialisableT::* field, RestT SerialisableT::*... rest)
			{
				SerialiseField(field);

				if constexpr (sizeof...(rest) > 0)
					Serialise(rest...);
			}

			template<typename FieldT, typename... RestT>
			void Deserialise(FieldT SerialisableT::* field, RestT SerialisableT::*... rest)
			{
				DeserialiseField(field);

				if constexpr (sizeof...(rest) > 0)
					Deserialise(rest...);
			}

			void WriteToSerial(Serial* serial)
			{
				ionassert(m_Type == EType::WRITE);
				ionassert(m_BytesCounter <= sizeof(SerialisableT));

				if (m_State != EState::SERIALISING)
				{
					CoreLogger.Critical("Cannot write to serial in this state! {0}", m_State);
					ionassert(false);
					return;
				}
				m_State = EState::WRITING;

				ionassert(m_Bytes);

				uint8* shrunkBuffer = new uint8[m_BytesCounter];
				memcpy_s(shrunkBuffer, m_BytesCounter, m_Bytes, m_BytesCounter);
				serial->PushBytes(shrunkBuffer, m_BytesCounter);
			}

			void ReadFromSerial(Serial* serial)
			{
				ionassert(m_Type == EType::READ);
				ionassert(m_BytesCounter <= sizeof(SerialisableT));

				if (!(m_State == EState::INIT))
				{
					CoreLogger.Critical("Cannot read from serial in this state! {0}", m_State);
					ionassert(false);
					return;
				}
				m_State = EState::READING;

				ionassert(!m_Bytes);

				uint64 pulledBytes;
				m_Bytes = serial->PullBytes(&pulledBytes);
				m_MaxCount = pulledBytes;
			}

			void Finalise()
			{
				if (!(m_State == EState::WRITING || m_State == EState::DESERIALISING))
				{
					CoreLogger.Critical("Cannot finalise serialisation in this state! {0}", m_State);
					ionassert(false);
					return;
				}
				m_State = EState::FINALISED;

				delete[] m_Bytes;
				m_Bytes = nullptr;
				m_BytesCounter = 0;
				m_MaxCount = 0;
			}

		protected:
			template<typename FieldT>
			void SerialiseField(FieldT SerialisableT::* field)
			{
				if (!CheckSerialise())
					return;

				m_State = EState::SERIALISING;

				if constexpr (TIsBaseOfV<ISerialisable, FieldT>)
				{
					// @TODO: Make it so this recursive serialisation doesn't actually store the size in the serial

					FieldT* fieldPtr = &(m_SerialisablePtr->*field);
					ISerialisable* serialisableField = (ISerialisable*)fieldPtr;

					Serial serial;
					serialisableField->Serialise(&serial);

					uint8* serialBuffer = serial.CreateSignedBuffer();
					uint64 sizeToWrite = serial.GetSize() + LengthSize;
					memcpy_s(m_Bytes + m_BytesCounter, sizeToWrite, serialBuffer, sizeToWrite);
					delete[] serialBuffer;

					m_BytesCounter += sizeToWrite;
				}
				else
				{
					uint64 fieldSize = sizeof(FieldT);
					FieldT& data = m_SerialisablePtr->*field;
					memcpy_s(m_Bytes + m_BytesCounter, fieldSize, &data, fieldSize);
					m_BytesCounter += fieldSize;
				}
			}

			template<typename FieldT>
			void DeserialiseField(FieldT SerialisableT::* field)
			{
				if (!CheckDeserialise())
					return;

				m_State = EState::DESERIALISING;

				if constexpr (TIsBaseOfV<ISerialisable, FieldT>)
				{
					FieldT* fieldPtr = &(m_SerialisablePtr->*field);
					ISerialisable* serialisableField = (ISerialisable*)fieldPtr;

					Serial serial;
					uint64 serialSize;
					memcpy_s(&serialSize, LengthSize, m_Bytes + m_BytesCounter, LengthSize);

					uint8* buffer = new uint8[serialSize];
					memcpy_s(buffer, serialSize, m_Bytes + m_BytesCounter + LengthSize, serialSize);
					serial.PushBytes(buffer, serialSize);
					serialisableField->Deserialise(&serial);

					m_BytesCounter += serialSize + LengthSize;
				}
				else
				{
					uint64 fieldSize = sizeof(FieldT);
					FieldT& data = m_SerialisablePtr->*field;
					memcpy_s(&data, fieldSize, m_Bytes + m_BytesCounter, fieldSize);
					m_BytesCounter += fieldSize;
				}
			}

		private:
			bool CheckSerialise()
			{
				ionassert(m_Type == EType::WRITE);

				if (!(m_State == EState::INIT || m_State == EState::SERIALISING))
				{
					CoreLogger.Critical("Cannot serialise field in this state! {0}", m_State);
					ionassert(false);
					return false;
				}
				
				ionassert(m_Bytes);
				return true;
			}

			bool CheckDeserialise()
			{
				ionassert(m_Type == EType::READ);

				if (!(m_State == EState::READING || m_State == EState::DESERIALISING))
				{
					CoreLogger.Critical("Cannot deserialise field in this state! {0}", m_State);
					ionassert(false);
					return false;
				}

				ionassert(m_Bytes);
				return true;
			}

		private:
			EType m_Type;
			EState m_State;
			SerialisableT* m_SerialisablePtr;
			uint64 m_BytesCounter;
			uint64 m_MaxCount;
			uint8* m_Bytes;
		};

		template<typename T>
		void SerialiseStruct(T* structPtr, Serial* serial)
		{
			uint64 size = sizeof(T);
			uint8* bytes = new uint8[size];
			memcpy_s(bytes, size, structPtr, size);
			serial->PushBytes(bytes, size);
		}

		template<typename T>
		void DeserialiseStruct(T* structPtr, Serial* serial)
		{
			uint64 size = sizeof(T);
			uint64 pulledSize;
			uint8* bytes = serial->PullBytes(&pulledSize);
			ionassert(size == pulledSize);
			memcpy_s(structPtr, size, bytes, pulledSize);
			delete[] bytes;
		}
	}

	using ISerialisable = Serialisation::ISerialisable;
	using Serial = Serialisation::Serial;

	struct SerialTest
	{
		float a = 0.0f;
		int32 b = 0;
		long c = 0l;
		uint32 d = 0u;
	};

	class SerialClassTest : public ISerialisable
	{
	public:
		float a = 1.0f;
		int32 b = -2;
		long c[128];
		uint32 d = 45u;

		virtual void Serialise(Serialisation::Serial* serial) override
		{
			Serialisation::TClassSerialiser serialiser(this, Serialisation::EType::WRITE);
			serialiser.Serialise(
				&SerialClassTest::a,
				&SerialClassTest::b,
				&SerialClassTest::c,
				&SerialClassTest::d
			);
			serialiser.WriteToSerial(serial);
			serialiser.Finalise();
		}

		virtual void Deserialise(Serialisation::Serial* serial) override
		{
			Serialisation::TClassSerialiser serialiser(this, Serialisation::EType::READ);
			serialiser.ReadFromSerial(serial);
			serialiser.Deserialise(
				&SerialClassTest::a,
				&SerialClassTest::b,
				&SerialClassTest::c,
				&SerialClassTest::d
			);
			serialiser.Finalise();
		}
	};

	class SerialClass2 : public ISerialisable
	{
	public:
		int32 primitive = 2;
		SerialClassTest serialisable;

	public:
		virtual void Serialise(Serialisation::Serial* serial) override
		{
			Serialisation::TClassSerialiser serialiser(this, Serialisation::EType::WRITE);
			serialiser.Serialise(
				&SerialClass2::primitive,
				&SerialClass2::serialisable
			);
			serialiser.WriteToSerial(serial);
			serialiser.Finalise();
		}

		virtual void Deserialise(Serialisation::Serial* serial) override
		{
			Serialisation::TClassSerialiser serialiser(this, Serialisation::EType::READ);
			serialiser.ReadFromSerial(serial);
			serialiser.Deserialise(
				&SerialClass2::primitive,
				&SerialClass2::serialisable
			);
			serialiser.Finalise();
		}
	};

	class SerialClass3 : public ISerialisable
	{
	public:
		SerialClass2 serialisableDeep;

		virtual void Serialise(Serialisation::Serial* serial) override
		{
			Serialisation::TClassSerialiser serialiser(this, Serialisation::EType::WRITE);
			serialiser.Serialise(&SerialClass3::serialisableDeep);
			serialiser.WriteToSerial(serial);
			serialiser.Finalise();
		}

		virtual void Deserialise(Serialisation::Serial* serial) override
		{
			Serialisation::TClassSerialiser serialiser(this, Serialisation::EType::READ);
			serialiser.ReadFromSerial(serial);
			serialiser.Deserialise(&SerialClass3::serialisableDeep);
			serialiser.Finalise();
		}
	};

	inline void SerialisationTest()
	{
		SerialClassTest t;
		t.a = 93.2f;
		t.b = -51;
		memset(t.c, 0, sizeof(t.c));
		t.d = 666601;

		Serial serial;
		t.Serialise(&serial);

		SerialClassTest t2;
		t2.Deserialise(&serial);

		ionassert(t2.a == t.a);
		ionassert(t2.b == t.b);
		ionassert(memcmp(t2.c, t.c, 128) == 0);
		ionassert(t2.d == t.d);

		using namespace std::chrono_literals;
		// Memory leak test (passed)
		for (int32 i = 0; i < 100000; ++i)
		{
			SerialClassTest ct;
			ct.a = 93.2f;
			ct.b = -51;
			ct.d = 666601;

			Serialisation::Serial cserial;
			ct.Serialise(&cserial);

			SerialClassTest ct2;
			ct2.Deserialise(&cserial);

			ionassert(ct2.a == ct.a);
			ionassert(ct2.b == ct.b);
			ionassert(ct2.d == ct.d);
		}

		SerialClass2 tt;
		tt.serialisable.a = 6.8f;
		tt.serialisable.b = -5556;
		tt.serialisable.d = 66;
		
		tt.Serialise(&serial);
		
		SerialClass2 tt2;
		tt2.Deserialise(&serial);

		// Memory leak test (passed)
		for (int32 i = 0; i < 100000; ++i)
		{
			SerialClass3 ttt;
			ttt.serialisableDeep.primitive = 888;
			ttt.serialisableDeep.serialisable.a = 66.82f;
			ttt.serialisableDeep.serialisable.b = -11111;
			ttt.serialisableDeep.serialisable.d = 9210;
			ttt.Serialise(&serial);

			SerialClass3 ttt2;
			ttt2.Deserialise(&serial);
		}

		// Memory leak test (passed)
		for (int32 i = 0; i < 100000; ++i)
		{
			Serial structSerial;
			SerialTest st1;
			st1.a = 646.3423f;
			st1.b = -2323;
			st1.c = 10000000;
			st1.d = 111111;
			Serialisation::SerialiseStruct(&st1, &structSerial);

			SerialTest st2;
			Serialisation::DeserialiseStruct(&st2, &structSerial);
		}
	}
}
