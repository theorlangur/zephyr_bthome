#ifndef BTHHOME_TYPES_HPP_
#define BTHHOME_TYPES_HPP_

#include "bthome_tools.hpp"

namespace BTHome
{
    template<class T>
    concept IsBTHomeDataType = requires { T::kDataId; T::kDataSize; };

    template<uint8_t Id, uint8_t DataSize, class ConcreteDataType>
    struct Data
    {
        static const constexpr uint8_t kDataId = Id;
        static const constexpr uint8_t kDataSize = DataSize;

        constexpr Data(auto v):
            m_Data{Id, 0}
        {
            ConcreteDataType::convert_from(v, m_Data + 1);
        }

        constexpr void fill(uint8_t *pDst) const
        {
            for(size_t i = 0; i < (ConcreteDataType::kDataSize + 1); ++i)
                pDst[i] = m_Data[i];
        }

        uint8_t m_Data[DataSize + 1];
    };

    struct bth_type_t {using bth_type_tag = void;};
    struct bth_uint8_t :bth_type_t{ using real_t = uint8_t; uint8_t d; };
    struct bth_uint16_t:bth_type_t{ using real_t = uint16_t; uint8_t d[2]; };
    struct bth_uint24_t:bth_type_t{ using real_t = uint32_t; uint8_t d[3]; };
    struct bth_uint32_t:bth_type_t{ using real_t = uint32_t; uint8_t d[4]; };

    struct bth_sint8_t :bth_type_t{ using real_t = int8_t; int8_t d; };
    struct bth_sint16_t:bth_type_t{ using real_t = int16_t; int8_t d[2]; };
    struct bth_sint24_t:bth_type_t{ using real_t = int32_t; int8_t d[3]; };
    struct bth_sint32_t:bth_type_t{ using real_t = int32_t; int8_t d[4]; };

    template<class T>
    concept IsBTHomeType = requires { typename T::bth_type_tag; };

    template<uint8_t Id, IsBTHomeType DataType, float f>
    struct FloatData: Data<Id, sizeof(DataType), FloatData<Id, DataType, f>>
    {
        using Parent = Data<Id, sizeof(DataType), FloatData<Id, DataType, f>>;
        constexpr FloatData(float t = {}):Parent(t){}

        static constexpr void convert_from(float t, uint8_t *pDst)
        {
            uint32_t v = uint32_t(t * f);
            for(uint8_t i  = 0; i < sizeof(DataType); ++i)
            {
                pDst[i] = v & 0xff;
                v >>= 8;
            }
        }
    };

    template<uint8_t Id, IsBTHomeType DataType>
    struct IntData: Data<Id, sizeof(DataType), IntData<Id, DataType>>
    {
        using Parent = Data<Id, sizeof(DataType), IntData<Id, DataType>>;
        constexpr IntData(DataType::real_t t = {}):Parent(t){}

        static constexpr void convert_from(DataType::real_t v, uint8_t *pDst)
        {
            for(uint8_t i  = 0; i < sizeof(DataType); ++i)
            {
                pDst[i] = v & 0xff;
                v >>= 8;
            }
        }
    };

    template<uint8_t Id, class Binary = bool>
    struct BinaryData: Data<Id, 1, BinaryData<Id, Binary>>
    {
        using Parent = Data<Id, 1, BinaryData<Id, Binary>>;
        constexpr BinaryData(Binary t):Parent(t){}

        static constexpr void convert_from(Binary v, uint8_t *pDst)
        {
            pDst[0] = bool(v) ? 1 : 0;
        }
    };

    template<uint8_t Id, class EData>
    struct EnumData: Data<Id, 1, EnumData<Id, EData>>
    {
        using Parent = Data<Id, 1, EnumData<Id, EData>>;
        constexpr EnumData(EData t = {}):Parent(t){}

        static constexpr void convert_from(EData v, uint8_t *pDst)
        {
            pDst[0] = uint8_t(v);
        }
    };

    struct Acceleration:       FloatData<0x51/*Id*/, bth_uint16_t, 1000.f/*Factor*/> { using FloatData::FloatData; };
    struct AccelerationSigned: FloatData<0x63/*Id*/, bth_sint32_t, 100000.f/*Factor*/> { using FloatData::FloatData; };
    struct Battery:              IntData<0x01/*Id*/, bth_uint8_t> { using IntData::IntData; };
    struct Channel:              IntData<0x60/*Id*/, bth_uint8_t> { using IntData::IntData; };
    struct CO2:                  IntData<0x12/*Id*/, bth_uint16_t> { using IntData::IntData; };
    struct Conductivity:         IntData<0x56/*Id*/, bth_uint16_t> { using IntData::IntData; };
    struct Count1:               IntData<0x09/*Id*/, bth_uint8_t> { using IntData::IntData; };
    struct Count2:               IntData<0x3D/*Id*/, bth_uint16_t> { using IntData::IntData; };
    struct Count4:               IntData<0x3E/*Id*/, bth_uint32_t> { using IntData::IntData; };
    struct CountSigned1:         IntData<0x59/*Id*/, bth_sint8_t> { using IntData::IntData; };
    struct CountSigned2:         IntData<0x5A/*Id*/, bth_sint16_t> { using IntData::IntData; };
    struct CountSigned4:         IntData<0x5B/*Id*/, bth_sint32_t> { using IntData::IntData; };
    struct Current:            FloatData<0x43/*Id*/, bth_uint16_t, 1000.f/*Factor*/> { using FloatData::FloatData; };
    struct CurrentSigned:      FloatData<0x5D/*Id*/, bth_sint16_t, 1000.f/*Factor*/> { using FloatData::FloatData; };
    struct Dewpoint:           FloatData<0x08/*Id*/, bth_uint16_t, 100.f/*Factor*/> { using FloatData::FloatData; };
    struct Direction:          FloatData<0x5e/*Id*/, bth_sint16_t, 100.f/*Factor*/> { using FloatData::FloatData; };
    struct DistanceMM:           IntData<0x41/*Id*/, bth_uint16_t> { using IntData::IntData; };
    struct DistanceM:          FloatData<0x41/*Id*/, bth_uint16_t, 10.f/*Factor*/> { using FloatData::FloatData; };
    struct Duration:           FloatData<0x42/*Id*/, bth_uint24_t, 1000.f/*Factor*/> { using FloatData::FloatData; };
    struct Energy:             FloatData<0x4D/*Id*/, bth_uint32_t, 1000.f/*Factor*/> { using FloatData::FloatData; };
    struct Energy3:            FloatData<0x0A/*Id*/, bth_uint24_t, 1000.f/*Factor*/> { using FloatData::FloatData; };
    struct Gas:                FloatData<0x4C/*Id*/, bth_uint32_t, 1000.f/*Factor*/> { using FloatData::FloatData; };
    struct Gas3:               FloatData<0x4B/*Id*/, bth_uint24_t, 1000.f/*Factor*/> { using FloatData::FloatData; };
    struct Gyroscope:          FloatData<0x52/*Id*/, bth_uint16_t, 1000.f/*Factor*/> { using FloatData::FloatData; };
    struct Humidity:           FloatData<0x03/*Id*/, bth_uint16_t, 100.f/*Factor*/> { using FloatData::FloatData; };
    struct Humidity1:            IntData<0x2E/*Id*/, bth_uint8_t> { using IntData::IntData; };
    struct Illuminance:        FloatData<0x05/*Id*/, bth_uint24_t, 100.f/*Factor*/> { using FloatData::FloatData; };
    struct MassKg:             FloatData<0x06/*Id*/, bth_uint16_t, 100.f/*Factor*/> { using FloatData::FloatData; };
    struct MassLb:             FloatData<0x07/*Id*/, bth_uint16_t, 100.f/*Factor*/> { using FloatData::FloatData; };
    struct Moisture:           FloatData<0x14/*Id*/, bth_uint16_t, 100.f/*Factor*/> { using FloatData::FloatData; };
    struct Moisture1:            IntData<0x2F/*Id*/, bth_uint8_t> { using IntData::IntData; };
    struct PM2_5:                IntData<0x0D/*Id*/, bth_uint16_t>{ using IntData::IntData; };
    struct PM10:                 IntData<0x0E/*Id*/, bth_uint16_t>{ using IntData::IntData; };
    struct Power:              FloatData<0x0B/*Id*/, bth_uint24_t, 100.f/*Factor*/> { using FloatData::FloatData; };
    struct PowerSigned:        FloatData<0x5C/*Id*/, bth_sint32_t, 100.f/*Factor*/> { using FloatData::FloatData; };
    struct Precipitation:      FloatData<0x5F/*Id*/, bth_uint16_t, 10.f/*Factor*/> { using FloatData::FloatData; };
    struct Pressure:           FloatData<0x04/*Id*/, bth_uint24_t, 100.f/*Factor*/> { using FloatData::FloatData; };
    struct Rotation:           FloatData<0x3F/*Id*/, bth_sint16_t, 10.f/*Factor*/> { using FloatData::FloatData; };
    struct RotationalSpeed:      IntData<0x61/*Id*/, bth_uint16_t>{ using IntData::IntData; };
    struct Speed:              FloatData<0x44/*Id*/, bth_uint16_t, 100.f/*Factor*/> { using FloatData::FloatData; };
    struct SpeedSigned:        FloatData<0x62/*Id*/, bth_sint32_t, 100000.f/*Factor*/> { using FloatData::FloatData; };
    struct TemperatureCoarse:    IntData<0x57/*Id*/, bth_sint8_t> { using IntData::IntData; };
    struct Temperature35:      FloatData<0x58/*Id*/, bth_sint8_t, 1.f / 0.35f/*Factor*/> { using FloatData::FloatData; };
    struct TemperatureDec:     FloatData<0x45/*Id*/, bth_sint16_t, 10.f/*Factor*/> { using FloatData::FloatData; };
    struct Temperature:        FloatData<0x02/*Id*/, bth_sint16_t, 100.f/*Factor*/> { using FloatData::FloatData; };
    struct Timestamp:            IntData<0x50/*Id*/, bth_uint32_t> { using IntData::IntData; };
    struct TVOC:                 IntData<0x13/*Id*/, bth_uint16_t> { using IntData::IntData; };
    struct VoltageFine:        FloatData<0x0C/*Id*/, bth_uint16_t, 1000.f/*Factor*/> { using FloatData::FloatData; };
    struct VoltageCoarse:      FloatData<0x4A/*Id*/, bth_uint16_t, 10.f/*Factor*/> { using FloatData::FloatData; };
    struct VolumeFine:         FloatData<0x4E/*Id*/, bth_uint32_t, 1000.f/*Factor*/> { using FloatData::FloatData; };
    struct Volume:             FloatData<0x47/*Id*/, bth_uint16_t, 10.f/*Factor*/> { using FloatData::FloatData; };
    struct VolumeMilli:          IntData<0x48/*Id*/, bth_uint16_t> { using IntData::IntData; };
    struct VolumeStorage:      FloatData<0x55/*Id*/, bth_uint32_t, 1000.f/*Factor*/> { using FloatData::FloatData; };
    struct VolumeFloatRate:    FloatData<0x49/*Id*/, bth_uint16_t, 1000.f/*Factor*/> { using FloatData::FloatData; };
    struct UVIndex:            FloatData<0x46/*Id*/, bth_uint8_t, 10.f/*Factor*/> { using FloatData::FloatData; };
    struct Water:              FloatData<0x4F/*Id*/, bth_uint32_t, 1000.f/*Factor*/> { using FloatData::FloatData; };

    //TODO: raw, text

    //binary types
    enum class EBatteryState: uint8_t {Normal=0, Low=1};
    struct BatteryState:    BinaryData<0x15, EBatteryState> {using BinaryData::BinaryData;};
    struct BatteryCharging: BinaryData<0x16> {using BinaryData::BinaryData;};
    struct CODetection:     BinaryData<0x17> {using BinaryData::BinaryData;};
    struct Cold:            BinaryData<0x18> {using BinaryData::BinaryData;};
    enum class EConnectivity: uint8_t {Disconnected=0, Connected=1};
    struct Connectivity:    BinaryData<0x19, EConnectivity> {using BinaryData::BinaryData;};
    enum class EDoor: uint8_t {Closed=0, Open=1};
    struct Door:            BinaryData<0x1A, EDoor> {using BinaryData::BinaryData;};
    struct GarageDoor:      BinaryData<0x1B, EDoor> {using BinaryData::BinaryData;};
    struct GasDetection:    BinaryData<0x1C> {using BinaryData::BinaryData;};
    struct GenericBool:     BinaryData<0x0F> {using BinaryData::BinaryData;};
    enum class EHeat: uint8_t {Normal=0, Hot=1};
    struct Heat:            BinaryData<0x1D, EHeat> {using BinaryData::BinaryData;};
    struct Light:           BinaryData<0x1E> {using BinaryData::BinaryData;};
    enum class ELock: uint8_t {Locked=0, Unlocked=1};
    struct Lock:           BinaryData<0x1F, ELock> {using BinaryData::BinaryData;};
    enum class EMoisture: uint8_t {Locked=0, Unlocked=1};
    struct MoistureDetection: BinaryData<0x20, EMoisture> {using BinaryData::BinaryData;};
    struct Motion:          BinaryData<0x21> {using BinaryData::BinaryData;};
    struct Moving:          BinaryData<0x22> {using BinaryData::BinaryData;};
    struct Occupancy:       BinaryData<0x23> {using BinaryData::BinaryData;};
    struct Opennig:         BinaryData<0x11> {using BinaryData::BinaryData;};
    enum class EPlug: uint8_t {Unplugged=0, PluggedIn=1};
    struct Plug:           BinaryData<0x24, EPlug> {using BinaryData::BinaryData;};
    struct PowerDetected:  BinaryData<0x10> {using BinaryData::BinaryData;};
    struct Presence:       BinaryData<0x25> {using BinaryData::BinaryData;};
    struct Problem:        BinaryData<0x26> {using BinaryData::BinaryData;};
    struct Running:        BinaryData<0x27> {using BinaryData::BinaryData;};
    enum class ESafety: uint8_t {Unsafe=0, Safe=1};
    struct Safety:         BinaryData<0x28, ESafety> {using BinaryData::BinaryData;};
    struct Smoke:          BinaryData<0x29> {using BinaryData::BinaryData;};
    struct Sound:          BinaryData<0x2A> {using BinaryData::BinaryData;};
    struct Tamper:         BinaryData<0x2B> {using BinaryData::BinaryData;};
    struct Vibration:      BinaryData<0x2C> {using BinaryData::BinaryData;};
    enum class EWindow: uint8_t {Closed=0, Opened=1};
    struct Window:         BinaryData<0x2D, EWindow> {using BinaryData::BinaryData;};

    //events
    enum class EButton: uint8_t {None=0, Press=1, DoublePress=2, TriplePress=3, LongPress=4, LongDoublePress=5, LongTriplePress=6, HoldPress = 0x80};
    struct Button:         EnumData<0x3A, EButton> {using EnumData::EnumData;};

    enum class EDimmer: uint8_t {None=0, RotateLeft = 1, RotateRight=2};
    struct Dimmer:         EnumData<0x3C, EDimmer> {using EnumData::EnumData;};

    //dev id
    struct bth_fw3_t:bth_type_t{ using real_t = bth_fw3_t; uint8_t major = 0, minor = 0, sub = 0; };
    struct bth_fw4_t:bth_type_t{ using real_t = bth_fw4_t; uint8_t major = 0, minor = 0, sub = 0, sub2 = 0; };

    struct DeviceTypeId:   IntData<0xF0/*Id*/, bth_uint16_t> { using IntData::IntData; };
    struct DeviceFirmware4:   Data<0xF1, 4, DeviceFirmware4>
    {
        using Parent = Data<0xF1, 4, DeviceFirmware4>;
        constexpr DeviceFirmware4(bth_fw4_t t = {}):Parent(t){}

        static constexpr void convert_from(bth_fw4_t v, uint8_t *pDst)
        {
            pDst[0] = v.sub2;
            pDst[1] = v.sub;
            pDst[2] = v.minor;
            pDst[3] = v.major;
        }
    };
    struct DeviceFirmware3: Data<0xF2, 3, DeviceFirmware3>
    {
        using Parent = Data<0xF2, 3, DeviceFirmware3>;
        constexpr DeviceFirmware3(bth_fw3_t t = {}):Parent(t){}

        static constexpr void convert_from(bth_fw3_t v, uint8_t *pDst)
        {
            pDst[0] = v.sub;
            pDst[1] = v.minor;
            pDst[2] = v.major;
        }
    };
}
#endif
