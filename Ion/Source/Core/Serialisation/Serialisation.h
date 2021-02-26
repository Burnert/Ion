#pragma once

#include "Core/CoreApi.h"
#include "Core/CoreTypes.h"
#include "Core/Logging/Logger.h"

namespace Ion
{
	namespace Serialisation
	{
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
			virtual void Serialise(Serialisation::Serial* serial) = 0;
			/* Loads serialised data to the object it is called on */
			virtual void Deserialise(Serialisation::Serial* serial) = 0;
		};

		class ION_API Serial
		{
			template<typename SerialisableT, std::enable_if_t<std::is_base_of_v<ISerialisable, SerialisableT>, bool>>
			friend class Serialiser;

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

			ullong GetSize() const
			{
				return m_Size;
			}

			const byte* const GetImmutableBytes() const
			{
				return m_Bytes;
			}

		private:
			/* Sets pointer to bytes of this object to the one specified */
			void PushBytes(byte* bytes, ullong count)
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

			/* Returns bytes and uninitialises the pointer in this object */
			byte* PullBytes(ullong* outCount)
			{
				if (m_Initialised)
				{
					if (!m_Bytes)
						return nullptr;

					*outCount = m_Size;
					byte* temp = m_Bytes;
					m_Bytes = nullptr;
					m_Size = 0;
					m_Initialised = false;
					return temp;
				}
				outCount = 0;
				return nullptr;
			}

			/* Creates a buffer of this Serial's bytes preceded by the size of it */
			byte* CreateSignedBuffer()
			{
				if (m_Initialised && m_Size > 0)
				{
					byte* buffer = new byte[m_Size + sizeof(ullong)];
					memcpy_s(buffer, sizeof(ullong), &m_Size, sizeof(ullong));
					memcpy_s(buffer + sizeof(ullong), m_Size, m_Bytes, m_Size);
					return buffer;
				}
				return nullptr;
			}

		private:
			bool m_Initialised;
			ullong m_Size;
			byte* m_Bytes;
		};

		// @TODO: Write docs ASAP...

		template<typename SerialisableT, std::enable_if_t<std::is_base_of_v<ISerialisable, SerialisableT>, bool> = true>
		class Serialiser
		{
		public:
			Serialiser(SerialisableT* serialisable, EType type) :
				m_Type(type),
				m_SerialisablePtr(serialisable),
				m_BytesCounter(0),
				m_Count(0),
				m_State(EState::INIT)
			{
				if (type == EType::WRITE)
				{
					// @TODO: Make the buffer resizable instead of this nonsense 64 here.
					m_Bytes = new byte[sizeof(SerialisableT) * 64];
				}
				else
				{
					m_Bytes = nullptr;
				}
			}

			Serialiser() :
				m_Type(EType::UNDEFINED),
				m_SerialisablePtr(nullptr),
				m_BytesCounter(0),
				m_Count(0),
				m_Bytes(nullptr),
				m_State(EState::NULLSTATE)
			{ }
			
			virtual ~Serialiser()
			{
				if (m_Bytes)
				{
					delete[] m_Bytes;
				}
				if (m_State != EState::FINALISED)
				{
					LOG_WARN("Serialiser was destroyed before Finalise() was called! This might be a bug.");
				}
			}

			void Init(SerialisableT* serialisable, EType type)
			{
				if (!(m_State == EState::NULLSTATE || m_State == EState::FINALISED))
				{
					LOG_CRITICAL("Cannot initialise serialiser in this state! {0}", m_State);
					ASSERT(false);
					return;
				}
				m_Type = type;
				m_SerialisablePtr = serialisable;
				m_BytesCounter = 0;
				m_Count = 0;
				m_Bytes = new byte[sizeof(SerialisableT)];
				m_State = EState::INIT;
			}

			template<typename FieldT>
			void SerialiseField(FieldT SerialisableT::* field)
			{
				if (!CheckSerialise())
					return;

				m_State = EState::SERIALISING;

				if (std::is_base_of_v<ISerialisable, FieldT>)
				{
					FieldT* fieldPtr = &(m_SerialisablePtr->*field);
					ISerialisable* serialisableField = (ISerialisable*)fieldPtr;

					Serial serial;
					serialisableField->Serialise(&serial);

					byte* serialBuffer = serial.CreateSignedBuffer();
					ullong sizeToWrite = serial.GetSize() + sizeof(ullong);
					memcpy_s(m_Bytes + m_BytesCounter, sizeToWrite, serialBuffer, sizeToWrite);
					delete serialBuffer;

					m_BytesCounter += sizeToWrite;
				}
				else
				{
					ullong fieldSize = sizeof(FieldT);
					FieldT& data = m_SerialisablePtr->*field;
					memcpy_s(m_Bytes + m_BytesCounter, fieldSize, &data, fieldSize);
					m_BytesCounter += fieldSize;

				}

				
			}

			// @TODO: Add recursive ISerialisable check procedure.

			//template<typename FieldT>
			//void SerialiseField(FieldT SerialisableT::* field)
			//{
			//	if (!CheckSerialise())
			//		return;

			//	m_State = EState::SERIALISING;

			//	
			//}

			template<typename FieldT>
			void DeserialiseField(FieldT SerialisableT::* field)
			{
				if (!CheckDeserialise())
					return;

				m_State = EState::DESERIALISING;

				if (std::is_base_of_v<ISerialisable, FieldT>)
				{
					FieldT* fieldPtr = &(m_SerialisablePtr->*field);
					ISerialisable* serialisableField = (ISerialisable*)fieldPtr;

					Serial serial;
					ullong serialSize;
					memcpy_s(&serialSize, sizeof(ullong), m_Bytes + m_BytesCounter, sizeof(ullong));

					byte* buffer = new byte[serialSize];
					memcpy_s(buffer, serialSize, m_Bytes + m_BytesCounter + sizeof(ullong), serialSize);
					serial.PushBytes(buffer, serialSize);
					serialisableField->Deserialise(&serial);

					m_BytesCounter += serialSize + sizeof(ullong);
				}
				else
				{
					ullong fieldSize = sizeof(FieldT);
					FieldT& data = m_SerialisablePtr->*field;
					memcpy_s(&data, fieldSize, m_Bytes + m_BytesCounter, fieldSize);
					m_BytesCounter += fieldSize;
				}

				
			}

			//template<typename FieldT>
			//void DeserialiseField(ISerialisable SerialisableT::* field)
			//{
			//	if (!CheckDeserialise())
			//		return;

			//	m_State = EState::DESERIALISING;

			//	
			//}

			void WriteToSerial(Serial* serial)
			{
				ASSERT(m_Type == EType::WRITE);
				ASSERT(m_BytesCounter <= sizeof(SerialisableT));

				if (m_State != EState::SERIALISING)
				{
					LOG_CRITICAL("Cannot write to serial in this state! {0}", m_State);
					ASSERT(false);
					return;
				}
				m_State = EState::WRITING;

				ASSERT(m_Bytes)

				byte* shrunkBuffer = new byte[m_BytesCounter];
				memcpy_s(shrunkBuffer, m_BytesCounter, m_Bytes, m_BytesCounter);
				serial->PushBytes(shrunkBuffer, m_BytesCounter);
			}

			void ReadFromSerial(Serial* serial)
			{
				ASSERT(m_Type == EType::READ);
				ASSERT(m_BytesCounter <= sizeof(SerialisableT));

				if (!(m_State == EState::INIT))
				{
					LOG_CRITICAL("Cannot read from serial in this state! {0}", m_State);
					ASSERT(false);
					return;
				}
				m_State = EState::READING;

				ASSERT(!m_Bytes)

				ullong pulledBytes;
				m_Bytes = serial->PullBytes(&pulledBytes);
				m_Count = pulledBytes;
			}

			void Finalise()
			{
				if (!(m_State == EState::WRITING || m_State == EState::DESERIALISING))
				{
					LOG_CRITICAL("Cannot finalise serialisation in this state! {0}", m_State);
					ASSERT(false);
					return;
				}
				m_State = EState::FINALISED;
				delete[] m_Bytes;
				m_Bytes = nullptr;
				m_BytesCounter = 0;
				m_Count = 0;
			}

		private:
			bool CheckSerialise()
			{
				ASSERT(m_Type == EType::WRITE);

				if (!(m_State == EState::INIT || m_State == EState::SERIALISING))
				{
					LOG_CRITICAL("Cannot serialise field in this state! {0}", m_State);
					ASSERT(false);
					return false;
				}
				
				ASSERT(m_Bytes)
				return true;
			}

			bool CheckDeserialise()
			{
				ASSERT(m_Type == EType::READ);

				if (!(m_State == EState::READING || m_State == EState::DESERIALISING))
				{
					LOG_CRITICAL("Cannot deserialise field in this state! {0}", m_State);
					ASSERT(false);
					return false;
				}

				ASSERT(m_Bytes)
				return true;
			}

		private:
			EType m_Type;
			EState m_State;
			SerialisableT* m_SerialisablePtr;
			ullong m_BytesCounter;
			ullong m_Count;
			byte* m_Bytes;
		};

		template<typename StructT>
		bool SerialiseStruct(StructT* structPtr, byte* serial)
		{
			ullong size = sizeof(StructT);
			memcpy_s(serial, size, structPtr, size);
		}
	}

	using ISerialisable = Serialisation::ISerialisable;
	using Serial = Serialisation::Serial;

	struct SerialTest
	{
		float a = 0.0f;
		int b = 0;
		long c = 0l;
		uint d = 0u;
	};

	class SerialClassTest : public ISerialisable
	{
	public:
		float a = 1.0f;
		int b = -2;
		long c[128];
		uint d = 45u;

		SerialClassTest()
		{
			ZeroMemory(c, sizeof(c));
		}

		virtual void Serialise(Serialisation::Serial* serial) override
		{
			Serialisation::Serialiser serialiser(this, Serialisation::EType::WRITE);
			serialiser.SerialiseField(&SerialClassTest::a);
			serialiser.SerialiseField(&SerialClassTest::b);
			serialiser.SerialiseField(&SerialClassTest::c);
			serialiser.SerialiseField(&SerialClassTest::d);
			serialiser.WriteToSerial(serial);
			serialiser.Finalise();
		}

		virtual void Deserialise(Serialisation::Serial* serial) override
		{
			Serialisation::Serialiser serialiser(this, Serialisation::EType::READ);
			serialiser.ReadFromSerial(serial);
			serialiser.DeserialiseField(&SerialClassTest::a);
			serialiser.DeserialiseField(&SerialClassTest::b);
			serialiser.DeserialiseField(&SerialClassTest::c);
			serialiser.DeserialiseField(&SerialClassTest::d);
			serialiser.Finalise();
		}
	};

	class SerialClass2 : public ISerialisable
	{
	public:
		int primitive = 2;
		SerialClassTest serialisable;

	public:
		virtual void Serialise(Serialisation::Serial* serial) override
		{
			Serialisation::Serialiser serialiser(this, Serialisation::EType::WRITE);
			serialiser.SerialiseField(&SerialClass2::primitive);
			serialiser.SerialiseField(&SerialClass2::serialisable);
			serialiser.WriteToSerial(serial);
			serialiser.Finalise();
		}

		virtual void Deserialise(Serialisation::Serial* serial) override
		{
			Serialisation::Serialiser serialiser(this, Serialisation::EType::READ);
			serialiser.ReadFromSerial(serial);
			serialiser.DeserialiseField(&SerialClass2::primitive);
			serialiser.DeserialiseField(&SerialClass2::serialisable);
			serialiser.Finalise();
		}
	};

	void SerialisationTest()
	{
		using namespace std::chrono_literals;
		// Memory leak test (passed)
		for (int i = 0; i < 100000; ++i)
		{
			SerialClassTest t;
			t.a = 93.2f;
			t.b = -51;
			//t.c = -55411;
			t.d = 666601;

			Serialisation::Serial serial;
			t.Serialise(&serial);

			SerialClassTest t2;
			t2.Deserialise(&serial);

			ASSERT(t2.a == t.a);
			ASSERT(t2.b == t.b);
			//ASSERT(t2.c == t.c);
			ASSERT(t2.d == t.d);
		}

		SerialClass2 tt;
		tt.serialisable.a = 6.8f;
		tt.serialisable.b = -5556;
		tt.serialisable.d = 66;
		
		Serial serial;
		tt.Serialise(&serial);
		
		SerialClass2 tt2;
		tt2.Deserialise(&serial);

		//LOG_INFO("SerialTest: {0}, {1}, {2}, {3}", t2.a, t2.b, t2.c, t2.d);
	}
}
