#include <motek/tradeup.hpp>

namespace motek {
TradeUp::TradeUp(size_t n, WearType avg_wear, bool st, SkinRarity r,
                 SkinCondition cond_no_st)
    : nSkins(n), average_wear(avg_wear), stattrak(st), rarity(r),
      condition_no_stattrak(cond_no_st), computed(false) {
  mask_vector.resize(nSkins);
  mask_big.resize(nSkins * 5);
  cost = grossreturn = netreturn = variance = stddev = vmr = profitchance = 0;

  Clear();
}

void TradeUp::Clear() {
  computed = false;
  cost = grossreturn = netreturn = variance = stddev = vmr = profitchance = 0;
  // mask.setZero();
}
} // namespace motek
