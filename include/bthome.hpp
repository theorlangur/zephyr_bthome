#ifndef BTHHOME_HPP_
#define BTHHOME_HPP_

#include <zephyr/bluetooth/bluetooth.h>
#include <iterator>
#include "bthome_tools.hpp"
#include "bthome_types.hpp"

namespace BTHome
{
    template<class... T> requires (IsBTHomeDataType<T> &&...)
    struct AdvertismentSVC
    {

        static const constexpr size_t kSVCDataSize = 
            2 /*ID 0xd2fc*/
            + 1 /*flags*/
            + sizeof...(T)//1 byte for data type
            + (T::kDataSize + ...)//however many bytes for actual types
        ;

        constexpr AdvertismentSVC(Flags f):
            m_SVCData{0xd2, 0xfc, f | kBTHomeVer, 0}
        {
            size_t off = 3;
            auto fill = [&](auto d)
            {
                d.fill(m_SVCData + off);
                off += 1 + d.kDataSize;
            };
            (fill(T()/*defaults*/),...);
        }

        constexpr AdvertismentSVC(Flags f, T... data):
            m_SVCData{0xd2, 0xfc, f | kBTHomeVer, 0}
        {
            size_t off = 3;
            auto fill = [&](auto d)
            {
                d.fill(m_SVCData + off);
                off += 1 + d.kDataSize;
            };
            (fill(data),...);
        }

        template<class X, class Value>
        void update(Value v)
        {
            static_assert(tools::DataOffset<1, X,T...>::kFound, "Data type not found");
            X::convert_from(v, m_SVCData + 3 + tools::DataOffset<1, X,T...>::kOffset + 1);
        }

        template<class X, size_t Nth, class Value>
        void update_nth(Value v)
        {
            static_assert(tools::DataOffset<Nth, X,T...>::kFound, "Data type not found");
            X::convert_from(v, m_SVCData + 3 + tools::DataOffset<Nth + 1, X,T...>::kOffset + 1);
        }

        uint8_t m_SVCData[kSVCDataSize];
    };

    template<class... T> requires (IsBTHomeDataType<T> &&...)
    AdvertismentSVC(Flags, T...) -> AdvertismentSVC<T...>;
    template<class... T> requires (IsBTHomeDataType<T> &&...)
    AdvertismentSVC(T...) -> AdvertismentSVC<T...>;

    struct AdvertisingPacket
    {
        template<size_t N, class... T>
        constexpr AdvertisingPacket(const char (&name)[N], AdvertismentSVC<T...> &SVC):
            m_Data{
                BT_DATA(BT_DATA_FLAGS, &g_Flags, 1),
                BT_DATA(BT_DATA_NAME_COMPLETE, name, N - 1),
                BT_DATA(BT_DATA_SVC_DATA16, SVC.m_SVCData, SVC.kSVCDataSize),
            }
        {
            constexpr size_t kTotalSize = 1/*flags*/ + (N - 1)/*name*/ + AdvertismentSVC<T...>::kSVCDataSize/*sensor types*/ + (sizeof(m_Data) / sizeof(m_Data[0])) * 2/*length byte + type byte*/;
            constexpr size_t kAllowedSensorPayload = 31 - (1/*flags*/ + (N - 1)/*name*/ + (sizeof(m_Data) / sizeof(m_Data[0])) * 2/*length byte + type byte*/);
            static_assert(kTotalSize <= 31, "Total size of advertisment data is too big!");
        }

        operator const bt_data* () const { return m_Data; }
        size_t size() const { return std::size(m_Data); }

        struct bt_data m_Data[3];

        inline static uint8_t g_Flags = BT_LE_AD_NO_BREDR | BT_LE_AD_GENERAL;
    };
};

#endif
