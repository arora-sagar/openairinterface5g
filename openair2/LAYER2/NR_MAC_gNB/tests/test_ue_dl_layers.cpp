/*
 * SPDX-License-Identifier: LicenseRef-CSSL-1.0
 */

#include "gtest/gtest.h"

extern "C" int test_ue_dl_layers(int n_prb, int endc, int endc_combo_in_nr_cap);

// UE: 4 DL layers, up to 100 MHz at 30 kHz. 189 PRB = 70 MHz, 273 PRB = 100 MHz
TEST(UeDlLayers, Sa70Mhz) { EXPECT_EQ(test_ue_dl_layers(189, 0, 0), 4); }
TEST(UeDlLayers, Sa100Mhz) { EXPECT_EQ(test_ue_dl_layers(273, 0, 0), 4); }
TEST(UeDlLayers, EnDc70Mhz) { EXPECT_EQ(test_ue_dl_layers(189, 1, 0), 4); }
TEST(UeDlLayers, EnDc100Mhz) { EXPECT_EQ(test_ue_dl_layers(273, 1, 0), 4); }
TEST(UeDlLayers, EnDc70MhzEnDcComboInNrCap) { EXPECT_EQ(test_ue_dl_layers(189, 1, 1), 4); }
