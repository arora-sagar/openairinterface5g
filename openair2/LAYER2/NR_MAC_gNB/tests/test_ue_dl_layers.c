/*
 * SPDX-License-Identifier: LicenseRef-CSSL-1.0
 */

/* Number of DL MIMO layers the gNB derives from the UE capability (ue_supported_dl_layers()),
 * for a UE supporting 4 layers up to 100 MHz, in SA and EN-DC. */

#include <stdlib.h>

#include "common/utils/oai_asn1.h"
#include "LAYER2/NR_MAC_COMMON/ue_capability_handler.h"
#include "LAYER2/NR_MAC_gNB/nr_radio_config.h"
#include "NR_ServingCellConfigCommon.h"
#include "NR_UE-NR-Capability.h"

static NR_BandParameters_t *band_nr(long band)
{
  NR_BandParameters_t *bp = calloc_or_fail(1, sizeof(*bp));
  bp->present = NR_BandParameters_PR_nr;
  bp->choice.nr = calloc_or_fail(1, sizeof(*bp->choice.nr));
  bp->choice.nr->bandNR = band;
  asn1cCallocOne(bp->choice.nr->ca_BandwidthClassDL_NR, NR_CA_BandwidthClassNR_a);
  asn1cCallocOne(bp->choice.nr->ca_BandwidthClassUL_NR, NR_CA_BandwidthClassNR_a);
  return bp;
}

static NR_BandParameters_t *band_eutra(long band)
{
  NR_BandParameters_t *bp = calloc_or_fail(1, sizeof(*bp));
  bp->present = NR_BandParameters_PR_eutra;
  bp->choice.eutra = calloc_or_fail(1, sizeof(*bp->choice.eutra));
  bp->choice.eutra->bandEUTRA = band;
  return bp;
}

static NR_FeatureSetsPerBand_t *fs_per_band(NR_FeatureSet_PR type)
{
  NR_FeatureSetsPerBand_t *fspb = calloc_or_fail(1, sizeof(*fspb));
  NR_FeatureSet_t *fs = calloc_or_fail(1, sizeof(*fs));
  fs->present = type;
  if (type == NR_FeatureSet_PR_nr) {
    fs->choice.nr = calloc_or_fail(1, sizeof(*fs->choice.nr));
    fs->choice.nr->downlinkSetNR = 1;
    fs->choice.nr->uplinkSetNR = 1;
  } else {
    fs->choice.eutra = calloc_or_fail(1, sizeof(*fs->choice.eutra));
    fs->choice.eutra->downlinkSetEUTRA = 1;
    fs->choice.eutra->uplinkSetEUTRA = 1;
  }
  asn1cSeqAdd(&fspb->list, fs);
  return fspb;
}

/* n78 UE, one DL/UL feature set: 30 kHz, up to 100 MHz, 4 DL layers. The NR band combination list
 * holds the n78 standalone combination and, if endc_combo_in_nr_cap, also an LTE B3 + n78 one. */
static NR_UE_NR_Capability_t *make_cap(bool endc_combo_in_nr_cap)
{
  NR_UE_NR_Capability_t *cap = calloc_or_fail(1, sizeof(*cap));
  cap->rf_Parameters.supportedBandCombinationList = calloc_or_fail(1, sizeof(*cap->rf_Parameters.supportedBandCombinationList));
  cap->featureSetCombinations = calloc_or_fail(1, sizeof(*cap->featureSetCombinations));

  NR_BandCombination_t *bc = calloc_or_fail(1, sizeof(*bc));
  asn1cSeqAdd(&bc->bandList.list, band_nr(78));
  bc->featureSetCombination = 0;
  asn1cSeqAdd(&cap->rf_Parameters.supportedBandCombinationList->list, bc);
  NR_FeatureSetCombination_t *fsc = calloc_or_fail(1, sizeof(*fsc));
  asn1cSeqAdd(&fsc->list, fs_per_band(NR_FeatureSet_PR_nr));
  asn1cSeqAdd(&cap->featureSetCombinations->list, fsc);

  if (endc_combo_in_nr_cap) {
    NR_BandCombination_t *bc2 = calloc_or_fail(1, sizeof(*bc2));
    asn1cSeqAdd(&bc2->bandList.list, band_eutra(3));
    asn1cSeqAdd(&bc2->bandList.list, band_nr(78));
    bc2->featureSetCombination = 1;
    asn1cSeqAdd(&cap->rf_Parameters.supportedBandCombinationList->list, bc2);
    NR_FeatureSetCombination_t *fsc2 = calloc_or_fail(1, sizeof(*fsc2));
    asn1cSeqAdd(&fsc2->list, fs_per_band(NR_FeatureSet_PR_eutra));
    asn1cSeqAdd(&fsc2->list, fs_per_band(NR_FeatureSet_PR_nr));
    asn1cSeqAdd(&cap->featureSetCombinations->list, fsc2);
  }

  NR_FeatureSets_t *fs = calloc_or_fail(1, sizeof(*fs));
  cap->featureSets = fs;
  fs->featureSetsDownlink = calloc_or_fail(1, sizeof(*fs->featureSetsDownlink));
  NR_FeatureSetDownlink_t *fsd = calloc_or_fail(1, sizeof(*fsd));
  NR_FeatureSetDownlinkPerCC_Id_t *dl_id = calloc_or_fail(1, sizeof(*dl_id));
  *dl_id = 1;
  asn1cSeqAdd(&fsd->featureSetListPerDownlinkCC.list, dl_id);
  asn1cSeqAdd(&fs->featureSetsDownlink->list, fsd);
  fs->featureSetsDownlinkPerCC = calloc_or_fail(1, sizeof(*fs->featureSetsDownlinkPerCC));
  NR_FeatureSetDownlinkPerCC_t *dlcc = calloc_or_fail(1, sizeof(*dlcc));
  dlcc->supportedSubcarrierSpacingDL = NR_SubcarrierSpacing_kHz30;
  dlcc->supportedBandwidthDL.present = NR_SupportedBandwidth_PR_fr1;
  dlcc->supportedBandwidthDL.choice.fr1 = NR_SupportedBandwidth__fr1_mhz100;
  asn1cCallocOne(dlcc->maxNumberMIMO_LayersPDSCH, NR_MIMO_LayersDL_fourLayers);
  asn1cSeqAdd(&fs->featureSetsDownlinkPerCC->list, dlcc);

  fs->featureSetsUplink = calloc_or_fail(1, sizeof(*fs->featureSetsUplink));
  NR_FeatureSetUplink_t *fsu = calloc_or_fail(1, sizeof(*fsu));
  NR_FeatureSetUplinkPerCC_Id_t *ul_id = calloc_or_fail(1, sizeof(*ul_id));
  *ul_id = 1;
  asn1cSeqAdd(&fsu->featureSetListPerUplinkCC.list, ul_id);
  asn1cSeqAdd(&fs->featureSetsUplink->list, fsu);
  fs->featureSetsUplinkPerCC = calloc_or_fail(1, sizeof(*fs->featureSetsUplinkPerCC));
  NR_FeatureSetUplinkPerCC_t *ulcc = calloc_or_fail(1, sizeof(*ulcc));
  ulcc->supportedSubcarrierSpacingUL = NR_SubcarrierSpacing_kHz30;
  ulcc->supportedBandwidthUL.present = NR_SupportedBandwidth_PR_fr1;
  ulcc->supportedBandwidthUL.choice.fr1 = NR_SupportedBandwidth__fr1_mhz100;
  asn1cSeqAdd(&fs->featureSetsUplinkPerCC->list, ulcc);
  return cap;
}

/* n78, 30 kHz SCS, n_prb PRBs */
static NR_ServingCellConfigCommon_t *make_scc(int n_prb)
{
  NR_ServingCellConfigCommon_t *scc = calloc_or_fail(1, sizeof(*scc));
  scc->downlinkConfigCommon = calloc_or_fail(1, sizeof(*scc->downlinkConfigCommon));
  NR_FrequencyInfoDL_t *fi = calloc_or_fail(1, sizeof(*fi));
  scc->downlinkConfigCommon->frequencyInfoDL = fi;
  NR_FreqBandIndicatorNR_t *band = calloc_or_fail(1, sizeof(*band));
  *band = 78;
  asn1cSeqAdd(&fi->frequencyBandList.list, band);
  NR_SCS_SpecificCarrier_t *car = calloc_or_fail(1, sizeof(*car));
  car->subcarrierSpacing = NR_SubcarrierSpacing_kHz30;
  car->carrierBandwidth = n_prb;
  asn1cSeqAdd(&fi->scs_SpecificCarrierList.list, car);
  return scc;
}

/* returns the DL layers derived by the gNB, -1 if none could be derived from the capability */
int test_ue_dl_layers(int n_prb, int endc, int endc_combo_in_nr_cap)
{
  NR_UE_NR_Capability_t *cap = make_cap(endc_combo_in_nr_cap);
  NR_ServingCellConfigCommon_t *scc = make_scc(n_prb);
  NR_feature_set_ids_t ids = feature_set_ids_handler(cap, scc->downlinkConfigCommon->frequencyInfoDL, endc ? EN_DC : NR_SA);
  long layers = ue_supported_dl_layers(scc, cap, ids.dl_feature_set_percc_id);
  ASN_STRUCT_FREE(asn_DEF_NR_UE_NR_Capability, cap);
  ASN_STRUCT_FREE(asn_DEF_NR_ServingCellConfigCommon, scc);
  return layers;
}
