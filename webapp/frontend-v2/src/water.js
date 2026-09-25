// Site Water Usage Effectiveness (WUE) for data center cooling - the
// U.S. average reported in:
//
//   Shehabi, A., et al. (2016). United States Data Center Energy Usage
//   Report. Lawrence Berkeley National Laboratory, LBNL-1005775.
//
// This is SITE cooling water only (the water evaporated on-site to cool
// the data center) - it explicitly excludes the water used upstream to
// generate the electricity itself, which is separately larger. Never
// present a figure derived from this constant as a total water
// footprint.
export const SITE_WUE_LITERS_PER_KWH = 1.8;

export const WUE_CITATION =
  "Site Water Usage Effectiveness (WUE): 1.8 L/kWh — U.S. average, Shehabi et al. (2016), " +
  "United States Data Center Energy Usage Report, Lawrence Berkeley National Laboratory, LBNL-1005775.";

// joules -> kWh (÷ 3,600,000, the definition of a kWh in joules) -> site
// cooling water in liters (× the WUE above). The one real, cited
// conversion this panel uses for every non-illustrative figure.
export function joulesToSiteCoolingLiters(joules) {
  return (joules / 3_600_000) * SITE_WUE_LITERS_PER_KWH;
}

// A demonstration multiplier only - not a research figure, not tied to
// any real deployment or workload. Chosen purely to make an otherwise
// invisible per-run quantity visible; label every figure derived from
// it as illustrative, never as measured or cited.
export const ILLUSTRATIVE_RUN_COUNT = 1_000_000_000;

// Formats a liters quantity at whatever metric unit keeps it in a
// legible 0.001-999 range, since these quantities span an enormous
// range (picoliters for one compile, up to milliliters at the
// illustrative billion-run scale).
export function formatLiters(liters) {
  const abs = Math.abs(liters);
  if (abs === 0) return "0 L";
  if (abs >= 1) return liters.toFixed(3) + " L";
  if (abs >= 1e-3) return (liters * 1e3).toFixed(3) + " mL";
  if (abs >= 1e-6) return (liters * 1e6).toFixed(3) + " µL";
  if (abs >= 1e-9) return (liters * 1e9).toFixed(3) + " nL";
  return (liters * 1e12).toFixed(3) + " pL";
}

// The one place every water-impact figure is computed, so the full
// /water page and the compact teaser on the Energy page can never drift
// apart on the numbers - both call this same function on the same
// energy/energyBefore data. Returns null if there's no energy total yet
// (nothing compiled), matching every other stage page's empty-state
// convention. `hasComparison` is false when there's no before-
// optimization figure to compare against (e.g. a result persisted from
// before this feature existed) - callers should hide anything
// before/after or percentage-based in that case, not show a broken 0%.
export function computeWaterImpact(energy, energyBefore) {
  const totalAfter = energy?.["Total Energy Units"];
  if (typeof totalAfter !== "number") return null;

  const totalBefore = energyBefore?.["Total Energy Units"];
  const hasComparison = typeof totalBefore === "number" && totalBefore > 0;

  const joulesAfter = totalAfter * 1e-9;
  const litersAfter = joulesToSiteCoolingLiters(joulesAfter);

  const joulesBefore = hasComparison ? totalBefore * 1e-9 : null;
  const litersBefore = hasComparison ? joulesToSiteCoolingLiters(joulesBefore) : null;
  const percentSaved = hasComparison ? ((totalBefore - totalAfter) / totalBefore) * 100 : null;

  const litersAfterAtScale = litersAfter * ILLUSTRATIVE_RUN_COUNT;
  const litersBeforeAtScale = hasComparison ? litersBefore * ILLUSTRATIVE_RUN_COUNT : null;
  const litersSavedAtScale = hasComparison ? litersBeforeAtScale - litersAfterAtScale : null;

  return {
    hasComparison,
    joulesAfter,
    litersAfter,
    joulesBefore,
    litersBefore,
    percentSaved,
    litersAfterAtScale,
    litersBeforeAtScale,
    litersSavedAtScale,
  };
}
