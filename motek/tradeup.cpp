#include <motek/tradeup.hpp>

namespace motek {
TradeUp::TradeUp(size_t nskins, wear_t avg_wear, bool stattrak,
                 SkinRarity rarity)
    : m_Computed(false), m_NSkins(nskins), m_AverageWear(avg_wear),
      m_StatTrak(stattrak), m_Rarity(rarity) {
  m_Mask.resize(m_NSkins);
  m_MaskBig.resize(m_NSkins * 5);
  m_Mask.reserve(10);
  m_MaskBig.reserve(50);

  Clear();
}

void TradeUp::Clear() {
  m_Computed = false;
  m_Cost = m_GrossReturn = m_NetReturn = 0;

  m_Mask.setZero();
  m_MaskBig.setZero();
}

void TradeUp::SetPrices(float cost, float grossreturn, float netreturn) {
  m_Cost = cost;
  m_GrossReturn = grossreturn;
  m_NetReturn = netreturn;
}

void TradeUp::SetAmount(int id, int amount) {
  m_Mask.insert(id) = static_cast<float>(amount);
}
void TradeUp::SetAmountCondition(int id, int amount,
                                 SkinCondition condition_no_stattrak) {
  m_MaskBig.insert(id * g_ConditionCount + condition_no_stattrak) =
      static_cast<float>(amount);
}

} // namespace motek
