#ifndef BTHOME_COMP_HPP
#define BTHOME_COMP_HPP
#include "bthome.hpp"

namespace BTHome
{
    namespace tools
    {
        template<class T>
        constexpr size_t DataSizeOf = (T::kDataSize + 1);

        template<class...T>
        constexpr size_t SumSize = 2 + 1 + (T::kDataSize + ...) + sizeof...(T);

        template<class... T>
        struct TypeList
        {
            static constexpr size_t kSize = sizeof...(T);
        };

        template<size_t SizeLimit, class Results, class CurrentBatch, class... Remaining>
        struct Packer;

        template<size_t SizeLimit, class... Results, class... Batch>
        struct Packer<SizeLimit, TypeList<Results...>, TypeList<Batch...>>
        {
            using type = TypeList<Results..., AdvertismentSVC<Batch...>>;
        };

        template<size_t SizeLimit, class... Results>
        struct Packer<SizeLimit, TypeList<Results...>, TypeList<>>
        {
            using type = TypeList<Results...>;
        };

        template<size_t SizeLimit, class... Results, class T, class... Batch, class... Rest>
        struct Packer<SizeLimit, TypeList<Results...>, TypeList<Batch...>, T, Rest...>
        {
            static_assert(SumSize<T> <= SizeLimit, "Data type is too big");
            using type = std::conditional_t<
                SumSize<T, Batch...> <= SizeLimit
                , typename Packer<SizeLimit, TypeList<Results...>, TypeList<Batch..., T>, Rest...>::type
                , typename Packer<SizeLimit, TypeList<Results..., AdvertismentSVC<Batch...>>, TypeList<T>, Rest...>::type
                >;
        };

        template<size_t Idx>
        struct index_tag_t{
            static constexpr size_t kIdx = Idx;
        };

        template<size_t Idx, class T>
        struct IndexedBase: T
        {
            using T::T;
            constexpr T& get(index_tag_t<Idx>) { return *this; }
        };

        template<class Indexes, class Types>
        struct MakeIndexedTypes;

        template<size_t... Idx, class... Types>
        struct MakeIndexedTypes<std::index_sequence<Idx...>, TypeList<Types...>>
        {
            using type = TypeList<IndexedBase<Idx, Types>...>;
        };

        template<class... AdvTypes>
        struct AdvertisementsListT: AdvTypes...
        {
            static constexpr size_t kSize = sizeof...(AdvTypes);
            using AdvTypes::get...;

            constexpr AdvertisementsListT(Flags f):AdvTypes{f}...
            {}
        };

        template<class AdvList>
        struct MakeAdvertisementList;

        template<class... AdvTypes>
        struct MakeAdvertisementList<TypeList<AdvTypes...>>
        {
            using type = AdvertisementsListT<AdvTypes...>;
        };

        template<size_t SizeLimit, class... DataTypes>
        using PackedAdvertisementsList = Packer<SizeLimit, TypeList<>, TypeList<>, DataTypes...>::type;

        template<class Packed>
        using AdvertisementsListFromPacked = MakeAdvertisementList<typename MakeIndexedTypes<decltype(std::make_index_sequence<Packed::kSize>()), Packed>::type>::type;

        template<size_t SizeLimit, class... DataTypes>
        using AdvertisementsList = AdvertisementsListFromPacked<PackedAdvertisementsList<SizeLimit, DataTypes...>>;

        template<size_t CurrentSize, class DataType>
        constexpr size_t DataTypeSizeWith = (CurrentSize == 0) ? SumSize<DataType> : CurrentSize + DataSizeOf<DataType>;

        template<size_t SizeLimit
            , size_t AdvIdx
            , size_t AdvDataOffset
            , size_t AdvSize
            , size_t DataIdx
            , class X, class... DataTypes>
        struct FindAdvIndexForType;

        template<size_t SizeLimit
            , size_t AdvIdx
            , size_t AdvDataOffset
            , size_t AdvSize
            , class X, class... DataTypes>
        struct FindAdvIndexForType<SizeLimit, AdvIdx, AdvDataOffset, AdvSize, 1, X, X, DataTypes...>
        {
            //found
            static constexpr size_t kNextAdvIndex = (DataTypeSizeWith<AdvSize, X> <= SizeLimit) ? AdvIdx : AdvIdx + 1;

            //resulting data
            static constexpr size_t kAdvertismentIndex = kNextAdvIndex;
            static constexpr size_t kAdvertismentDataOffset = AdvDataOffset;
            static constexpr bool kFound = true;
        };

        template<size_t SizeLimit
            , size_t AdvIdx
            , size_t AdvDataOffset
            , size_t AdvSize
            , size_t DataIdx
            , class X>
        struct FindAdvIndexForType<SizeLimit, AdvIdx, AdvDataOffset, AdvSize, DataIdx, X>
        {
            //didn't find anything
            //resulting data
            static constexpr size_t kAdvertismentIndex = 0xff;
            static constexpr size_t kAdvertismentDataOffset = 0xff;
            static constexpr bool kFound = false;
        };

        template<size_t SizeLimit
            , size_t AdvIdx
            , size_t AdvDataOffset
            , size_t AdvSize
            , size_t DataIdx
            , class X, class... DataTypes>
        struct FindAdvIndexForType<SizeLimit, AdvIdx, AdvDataOffset, AdvSize, DataIdx, X, X, DataTypes...>
        {
            //found some X but not our idx
            //helping data/types
            static constexpr size_t kNextAdvIndex = (DataTypeSizeWith<AdvSize, X> <= SizeLimit) ? AdvIdx : AdvIdx + 1;
            static constexpr size_t kNextAdvSize = DataTypeSizeWith<AdvSize, X> <= SizeLimit ? DataTypeSizeWith<AdvSize, X> : DataTypeSizeWith<0, X>;
            using next_type_t = FindAdvIndexForType<SizeLimit, kNextAdvIndex, AdvDataOffset + 1, kNextAdvSize, DataIdx - 1, X, DataTypes...>;

            //resulting data
            static constexpr size_t kAdvertismentIndex = next_type_t::kAdvertismentIndex;
            static constexpr size_t kAdvertismentDataOffset = next_type_t::kAdvertismentDataOffset;
            static constexpr bool kFound = next_type_t::kFound;
        };

        template<size_t SizeLimit
            , size_t AdvIdx
            , size_t AdvDataOffset
            , size_t AdvSize
            , size_t DataIdx
            , class X, class Y, class... DataTypes>
        struct FindAdvIndexForType<SizeLimit, AdvIdx, AdvDataOffset, AdvSize, DataIdx, X, Y, DataTypes...>
        {
            //not our type
            //helping data/types
            static constexpr size_t kNextAdvIndex = (DataTypeSizeWith<AdvSize, Y> <= SizeLimit) ? AdvIdx : AdvIdx + 1;
            static constexpr size_t kNextAdvSize = DataTypeSizeWith<AdvSize, Y> <= SizeLimit ? DataTypeSizeWith<AdvSize, Y> : DataTypeSizeWith<0, Y>;
            using next_type_t = FindAdvIndexForType<SizeLimit, kNextAdvIndex, AdvDataOffset, kNextAdvSize, DataIdx, X, DataTypes...>;

            //resulting data
            static constexpr size_t kAdvertismentIndex = next_type_t::kAdvertismentIndex;
            static constexpr size_t kAdvertismentDataOffset = next_type_t::kAdvertismentDataOffset;
            static constexpr bool kFound = next_type_t::kFound;
        };


        template<size_t SizeLimit, class Needle, class... Haystack>
        using FindDataTypeInAdvListT = FindAdvIndexForType<SizeLimit, 0, 0, 0, 1, Needle, Haystack...>;

        template<size_t SizeLimit, size_t Nth, class Needle, class... Haystack>
        using FindNthDataTypeInAdvListT = FindAdvIndexForType<SizeLimit, 0, 0, 0, 1 + Nth, Needle, Haystack...>;
    }

    template<size_t NameLen, class... T> requires (IsBTHomeDataType<T> &&...)
    struct Advertisement
    {
        static constexpr size_t kMaxAdvSize = 31;
        static constexpr size_t kAdvPacketFields = 3;
        static constexpr size_t kAllowedSensorPayload = kMaxAdvSize - (1/*flags*/ + (NameLen - 1) + kAdvPacketFields * 2/*length byte + type byte*/);

        using AdvDataHolder = tools::AdvertisementsList<kAllowedSensorPayload, T...>;
        static constexpr size_t kPacksCount =  AdvDataHolder::kSize;

        template<size_t N, class... S>
        constexpr Advertisement(const char (&name)[N], Flags f, S... datas):
            m_SensorData{f},
            m_Data{
                BT_DATA(BT_DATA_FLAGS, &g_Flags, 1),
                BT_DATA(BT_DATA_NAME_COMPLETE, name, N - 1),
                BT_DATA(BT_DATA_SVC_DATA16, nullptr, 0),
            }
        {
        }

        template<class X, class Value>
        void update(Value v)
        {
            using FindRes = tools::FindDataTypeInAdvListT<kAllowedSensorPayload, X, T...>;
            static_assert(FindRes::kFound, "Data type not found");
            auto &d = m_SensorData.get(tools::index_tag_t<FindRes::kAdvertismentIndex>{});
            d.template update<X, Value>(v);
        }

        template<class X, size_t Nth, class Value>
        void update_nth(Value v)
        {
            using FindRes = tools::FindNthDataTypeInAdvListT<kAllowedSensorPayload, Nth, X, T...>;
            static_assert(FindRes::kFound, "Data type not found");
            auto &d = m_SensorData.get(tools::index_tag_t<FindRes::kAdvertismentIndex>{});
            d.template update_nth<X, Nth - FindRes::kAdvertismentDataOffset, Value>(v);
        }

        void advertise()
        {
            const struct bt_le_adv_param adv_param[] = {
                BT_LE_ADV_PARAM_INIT(BT_LE_ADV_OPT_USE_IDENTITY, BT_GAP_ADV_SLOW_INT_MIN, BT_GAP_ADV_SLOW_INT_MAX, nullptr) 
            };
            advertise_with(adv_param, 1500);
        }

        void advertise_with(const bt_le_adv_param *adv_param, int adv_duration_ms)
        {
            auto &beg = m_SensorData.get(tools::index_tag_t<0>{});
            m_Data[kAdvPacketFields - 1].data = beg.m_SVCData;
            m_Data[kAdvPacketFields - 1].data_len = beg.kSVCDataSize;
            bt_le_adv_start(adv_param, m_Data, kAdvPacketFields, nullptr, 0);
            k_sleep(K_MSEC(adv_duration_ms));

            if constexpr (kPacksCount > 1)
            {
                auto update_adv = [&](auto index_tag)
                {
                    auto &d = m_SensorData.get(index_tag);
                    m_Data[kAdvPacketFields - 1].data = d.m_SVCData;
                    m_Data[kAdvPacketFields - 1].data_len = d.kSVCDataSize;
                    bt_le_adv_update_data(m_Data, kAdvPacketFields, nullptr, 0);
                    k_sleep(K_MSEC(adv_duration_ms));
                };

                [&]<size_t... Idx>(std::index_sequence<Idx...>)
                {
                    (update_adv(tools::index_tag_t<Idx + 1>{}),...);
                }(std::make_index_sequence<kPacksCount - 1>{});//<-- immediately invoke
            }

            bt_le_adv_stop();
        }

        AdvDataHolder m_SensorData;

        bt_data m_Data[kAdvPacketFields];
        inline static uint8_t g_Flags = BT_LE_AD_NO_BREDR | BT_LE_AD_GENERAL;
    };

    template<size_t N, class... T>
    Advertisement(const char (&name)[N], Flags f, T... Data) -> Advertisement<N, T...>;
}

#endif
