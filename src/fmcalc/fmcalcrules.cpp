/*
* Copyright (c)2015 - 2016 Oasis LMF Limited
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions
* are met:
*
*   * Redistributions of source code must retain the above copyright
*     notice, this list of conditions and the following disclaimer.
*
*   * Redistributions in binary form must reproduce the above copyright
*     notice, this list of conditions and the following disclaimer in
*     the documentation and/or other materials provided with the
*     distribution.
*
*   * Neither the original author of this software nor the names of its
*     contributors may be used to endorse or promote products derived
*     from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
* "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
* LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
* FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
* COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
* INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
* BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
* OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
* AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
* OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
* THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*/
/*

Author: Ben Matharu  email: ben.matharu@oasislmf.org

*/

#include "fmcalc.h"
#include <vector>



enum tc_new {			// tc = terms and conditions
	deductible_1,
	deductible_2,
	deductible_3,
	attachment_1,
	limit_1,
	share_1,
	share_2,
	share_3
};

void fmcalc::dofmcalc_new(std::vector <LossRec> &agg_vec, int layer)
{
	for (LossRec &x : agg_vec) {
		if (x.agg_id == 0) break;
		x.original_loss = x.loss;
		if (x.agg_id > 0) {
			if (x.policytc_id > 0) {
				const profile_rec_new &profile = profile_vec_new_[x.policytc_id];
				x.allocrule_id = allocrule_;
				switch (profile.calcrule_id) {
					case 1:
					{
						OASIS_FLOAT ded = 0;
						OASIS_FLOAT lim = 0;
						for (auto y : profile.tc_vec) {
							if (y.tc_id == deductible_1) ded = y.tc_val;
							if (y.tc_id == limit_1) lim = y.tc_val;
						}
						//Function1 = IIf(Loss < Ded, 0, IIf(Loss > Ded + Lim, Lim, Loss - Ded))
						OASIS_FLOAT loss = x.loss - ded;
						if (loss < 0) loss = 0;
						if (loss > lim) loss = lim;
						x.retained_loss = x.retained_loss + (x.loss - loss);
						if (layer >1)	x.net_loss = x.net_loss + (x.previous_layer_retained_loss - loss);
						else x.net_loss = x.retained_loss;
						x.loss = loss;
					}
					break;
					case 2:
					{
						OASIS_FLOAT ded = 0;
						OASIS_FLOAT lim = 0;
						OASIS_FLOAT share = 0;
						OASIS_FLOAT att = 0;
						for (auto y : profile.tc_vec) {
							if (y.tc_id == deductible_1) ded = y.tc_val;
							if (y.tc_id == limit_1) lim = y.tc_val;
							if (y.tc_id == share_1) share = y.tc_val;
							if (y.tc_id == attachment_1) att = y.tc_val;
						}
						//Function2: deductible applies before attachment limit share
						//IIf(Loss < Ded, 0, Loss - Ded)
						//IIf(Loss < Att, 0, IIf(Loss > Att + Lim, Lim, Loss - Att)) * Share	
						OASIS_FLOAT loss = 0;
						loss = x.loss - ded;
						if (loss < 0) loss = 0;
						if (loss > (att + lim)) loss = lim;
						else loss = loss - att;
						if (loss < 0) loss = 0;
						loss = loss * share;
						x.retained_loss = x.retained_loss + (x.loss - loss);
						if (layer >1)	x.net_loss = x.net_loss + (x.previous_layer_retained_loss - loss);
						else x.net_loss = x.retained_loss;
						x.loss = loss;
					}
					break;
					case 3:
					{
						OASIS_FLOAT ded = 0;
						OASIS_FLOAT lim = 0;
						for (auto y : profile.tc_vec) {
							if (y.tc_id == deductible_1) ded = y.tc_val;
							if (y.tc_id == limit_1) lim = y.tc_val;
						}
						//Function3 = IIf(Loss < Ded, 0, IIf(Loss > Ded, Lim, Loss))
						OASIS_FLOAT loss = x.loss;
						if (loss < ded) loss = 0;
						else loss = loss;
						if (loss > lim) loss = lim;
						x.retained_loss = x.retained_loss + (x.loss - loss);
						if (layer >1)	x.net_loss = x.net_loss + (x.previous_layer_retained_loss - loss);
						else x.net_loss = x.retained_loss;
						x.loss = loss;
					}
					break;
					case 4:
					{
						OASIS_FLOAT ded = 0;
						OASIS_FLOAT lim = 0;
						for (auto y : profile.tc_vec) {
							if (y.tc_id == deductible_1) ded = y.tc_val;
							if (y.tc_id == limit_1) lim = y.tc_val;
						}
						//Function4 = IIf(Loss < Ded * TIV, 0, IIf(Loss > Ded * TIV + Lim, Lim, Loss - Ded * TIV))
						OASIS_FLOAT loss = x.loss - (ded * x.accumulated_tiv);
						if (loss < 0) loss = 0;
						if (loss > lim) loss = lim;
						x.retained_loss = x.retained_loss + (x.loss - loss);
						if (layer >1)	x.net_loss = x.net_loss + (x.previous_layer_retained_loss - loss);
						else x.net_loss = x.retained_loss;
						x.loss = loss;
					}
					break;
					case 5:
					{
						OASIS_FLOAT ded = 0;
						OASIS_FLOAT lim = 0;
						for (auto y : profile.tc_vec) {
							if (y.tc_id == deductible_1) ded = y.tc_val;
							if (y.tc_id == limit_1) lim = y.tc_val;
						}
						//Function5 = Loss * (Lim - Ded)
						OASIS_FLOAT loss = x.loss * (lim - ded);
						x.retained_loss = x.retained_loss + (x.loss - loss);
						if (layer >1)	x.net_loss = x.net_loss + (x.previous_layer_retained_loss - loss);
						else x.net_loss = x.retained_loss;
						x.loss = loss;
					}
					break;
					case 6:
					{
						OASIS_FLOAT ded = 0;
						for (auto y : profile.tc_vec) {
							if (y.tc_id == deductible_1) ded = y.tc_val;
						}
						//Function6 = IIf(Loss < Ded * TIV, 0, Loss -(Ded * TIV))
						OASIS_FLOAT loss = x.loss - (ded * x.accumulated_tiv);
						if (loss < 0) loss = 0;
						x.retained_loss = x.retained_loss + (x.loss - loss);
						if (layer >1)	x.net_loss = x.net_loss + (x.previous_layer_retained_loss - loss);
						else x.net_loss = x.retained_loss;
						x.loss = loss;
					}
					break;
					case 7:
					{
						OASIS_FLOAT ded = 0;
						OASIS_FLOAT lim = 0;
						for (auto y : profile.tc_vec) {
							if (y.tc_id == deductible_3) ded = y.tc_val;
							if (y.tc_id == limit_1) lim = y.tc_val;
						}
						// Function 7: Applies a cap on retained loss (maximum deductible) and a limit
						OASIS_FLOAT loss = 0;
						if (x.retained_loss > ded) {
							loss = x.loss + x.retained_loss - ded;
							if (loss < 0) loss = 0;
							if (loss > lim) loss = lim;
							x.retained_loss = x.retained_loss + (x.loss - loss);
						}
						else {
							loss = x.loss;
							// retained loss stays the same
						}
						if (layer >1)	x.net_loss = x.net_loss + (x.previous_layer_retained_loss - loss);
						else x.net_loss = x.retained_loss;
						x.loss = loss;
					}
					break;
					case 8:
					{
						OASIS_FLOAT ded = 0;
						OASIS_FLOAT lim = 0;
						for (auto y : profile.tc_vec) {
							if (y.tc_id == deductible_2) ded = y.tc_val;
							if (y.tc_id == limit_1) lim = y.tc_val;
						}
						// Function 11: Applies a floor on retained loss (minimum deductible) and a limit
						OASIS_FLOAT loss = 0;
						if (x.retained_loss < ded) {
							loss = x.loss + x.retained_loss - ded;
							if (loss < 0) loss = 0;
							if (loss > lim) loss = lim;
							x.retained_loss = x.retained_loss + (x.loss - loss);
						}
						else {
							loss = x.loss;
							// retained loss stays the same
						}
						if (layer >1)	x.net_loss = x.net_loss + (x.previous_layer_retained_loss - loss);
						else x.net_loss = x.retained_loss;
						x.loss = loss;
					}
					break;
					case 9:
					{
						OASIS_FLOAT ded = 0;
						OASIS_FLOAT lim = 0;
						for (auto y : profile.tc_vec) {
							if (y.tc_id == deductible_1) ded = y.tc_val;
							if (y.tc_id == limit_1) lim = y.tc_val;
						}
						//Function9 = IIf(Loss < (Ded * lim), 0, IIf(Loss > (ded* lim) + Lim, Lim, Loss - (Ded * lim))
						OASIS_FLOAT loss = x.loss - (ded * lim);
						if (loss < 0) loss = 0;
						if (loss > lim) loss = lim;
						x.retained_loss = x.retained_loss + (x.loss - loss);
						if (layer >1)	x.net_loss = x.net_loss + (x.previous_layer_retained_loss - loss);
						else x.net_loss = x.retained_loss;
						x.loss = loss;
					}
					break;
					case 10:
					{
						OASIS_FLOAT ded = 0;
						for (auto y : profile.tc_vec) {
							if (y.tc_id == deductible_3) ded = y.tc_val;
						}
						// Function 10: Applies a cap on retained loss (maximum deductible)
						OASIS_FLOAT loss = 0;
						if (x.retained_loss > ded) {
							loss = x.loss + x.retained_loss - ded;
							if (loss < 0) loss = 0;
							x.retained_loss = x.retained_loss + (x.loss - loss);
						}
						else {
							loss = x.loss;
							// retained loss stays the same
						}
						if (layer >1)	x.net_loss = x.net_loss + (x.previous_layer_retained_loss - loss);
						else x.net_loss = x.retained_loss;
						x.loss = loss;
					}
					break;
					case 11:
					{
						OASIS_FLOAT ded = 0;
						for (auto y : profile.tc_vec) {
							if (y.tc_id == deductible_2) ded = y.tc_val;
						}
						// Function 11: Applies a floor on retained loss (minimum deductible)
						OASIS_FLOAT loss = 0;
						if (x.retained_loss < ded) {
							loss = x.loss + x.retained_loss - ded;
							if (loss < 0) loss = 0;
							x.retained_loss = x.retained_loss + (x.loss - loss);
						}
						else {
							loss = x.loss;
							// retained loss stays the same
						}
						if (layer >1)	x.net_loss = x.net_loss + (x.previous_layer_retained_loss - loss);
						else x.net_loss = x.retained_loss;
						x.loss = loss;
					}
					break;
					case 12:
					{
						for (auto &z : profile.tc_vec) {
							if (z.tc_id == deductible_1) {
								OASIS_FLOAT loss = x.loss - z.tc_val;
								if (loss < 0) loss = 0;
								x.retained_loss = x.retained_loss + (x.loss - loss);
								x.loss = loss;
								break;
							}
						}
					}
					break;
					case 13:
					{
						OASIS_FLOAT ded2 = 0;
						OASIS_FLOAT ded3 = 0;						
						for (auto y : profile.tc_vec) {
							if (y.tc_id == deductible_2) ded2 = y.tc_val;
							if (y.tc_id == deductible_3) ded3 = y.tc_val;
						}
						// Function 13: Applies a cap and floor on retained loss (minimum and maximum deductible)
						OASIS_FLOAT loss = 0;
						if (x.retained_loss > ded3) {
							loss = x.loss + x.retained_loss - ded3;
							if (loss < 0) loss = 0;
							x.retained_loss = x.retained_loss + (x.loss - loss);
						}
						if (x.retained_loss < ded2) {
							loss = x.loss + x.retained_loss - ded2;
							if (loss < 0) loss = 0;
							x.retained_loss = x.retained_loss + (x.loss - loss);
						}
						else {
							loss = x.loss;
							// retained loss stays the same
						}
						if (layer >1)	x.net_loss = x.net_loss + (x.previous_layer_retained_loss - loss);
						else x.net_loss = x.retained_loss;
						x.loss = loss;
					}
					break;
					case 14:
					{
						OASIS_FLOAT lim = 0;
						for (auto y : profile.tc_vec) {
							if (y.tc_id == limit_1) lim = y.tc_val;
						}
						//Function14 =  IIf(Loss > lim, Lim, Loss)
						OASIS_FLOAT loss = x.loss;
						if (loss > lim) loss = lim;
						x.retained_loss = x.retained_loss + (x.loss - loss);
						if (layer >1)	x.net_loss = x.net_loss + (x.previous_layer_retained_loss - loss);
						else x.net_loss = x.retained_loss;
						x.loss = loss;
					}
					break;
					case 15:
					{
						OASIS_FLOAT lim = 0;
						for (auto y : profile.tc_vec) {
							if (y.tc_id == limit_1) lim = y.tc_val;
						}
						//Function15 =  Loss * lim
						OASIS_FLOAT loss = x.loss;
						loss = loss * lim;
						x.retained_loss = x.retained_loss + (x.loss - loss);
						if (layer >1)	x.net_loss = x.net_loss + (x.previous_layer_retained_loss - loss);
						else x.net_loss = x.retained_loss;
						x.loss = loss;
					}
					break;
					case 16:
					{
						OASIS_FLOAT ded = 0;
						for (auto y : profile.tc_vec) {
							if (y.tc_id == deductible_1) ded = y.tc_val;
						}
						//Function16 =  Loss - (loss * ded)
						OASIS_FLOAT loss = x.loss;
						loss = loss - (loss * ded);
						if (loss < 0) loss = 0;
						x.retained_loss = x.retained_loss + (x.loss - loss);
						if (layer >1)	x.net_loss = x.net_loss + (x.previous_layer_retained_loss - loss);
						else x.net_loss = x.retained_loss;
						x.loss = loss;
					}
					break;
					case 17:
					{
						OASIS_FLOAT ded = 0;
						OASIS_FLOAT lim = 0;
						OASIS_FLOAT share = 0;
						OASIS_FLOAT att = 0;
						for (auto y : profile.tc_vec) {
							if (y.tc_id == deductible_1) ded = y.tc_val;
							if (y.tc_id == limit_1) lim = y.tc_val;
							if (y.tc_id == share_1) share = y.tc_val;
							if (y.tc_id == attachment_1) att = y.tc_val;
						}
						//Function17: deductible % loss applies before attachment limit share
						//IIf(Loss < Ded * loss, 0, Loss - (Ded * loss)
						//IIf(Loss < Att, 0, IIf(Loss > Att + Lim, Lim, Loss - Att)) * Share	
						OASIS_FLOAT loss = 0;
						loss = x.loss - (ded * x.loss);
						if (loss < 0) loss = 0;
						if (loss > (att + lim)) loss = lim;
						else loss = loss - att;
						if (loss < 0) loss = 0;
						loss = loss * share;
						x.retained_loss = x.retained_loss + (x.loss - loss);
						if (layer >1)	x.net_loss = x.net_loss + (x.previous_layer_retained_loss - loss);
						else x.net_loss = x.retained_loss;
						x.loss = loss;
					}
					break;
					case 18:
					{
						OASIS_FLOAT ded = 0;
						OASIS_FLOAT lim = 0;
						OASIS_FLOAT share = 0;
						OASIS_FLOAT att = 0;
						for (auto y : profile.tc_vec) {
							if (y.tc_id == deductible_1) ded = y.tc_val;
							if (y.tc_id == limit_1) lim = y.tc_val;
							if (y.tc_id == share_1) share = y.tc_val;
							if (y.tc_id == attachment_1) att = y.tc_val;
						}
						//Function18: deductible % tiv applies before attachment limit share
						//IIf(Loss < Ded * tiv, 0, Loss - (Ded * tiv)
						//IIf(Loss < Att, 0, IIf(Loss > Att + Lim, Lim, Loss - Att)) * Share	
						OASIS_FLOAT loss = 0;
						loss = x.loss - (ded * x.accumulated_tiv);
						if (loss < 0) loss = 0;
						if (loss > (att + lim)) loss = lim;
						else loss = loss - att;
						if (loss < 0) loss = 0;
						loss = loss * share;
						x.retained_loss = x.retained_loss + (x.loss - loss);
						if (layer >1)	x.net_loss = x.net_loss + (x.previous_layer_retained_loss - loss);
						else x.net_loss = x.retained_loss;
						x.loss = loss;
					}
					break;					
					case 19:
					{
						OASIS_FLOAT ded1 = 0;
						OASIS_FLOAT ded2 = 0;
						OASIS_FLOAT ded3 = 0;						
						for (auto y : profile.tc_vec) {
							if (y.tc_id == deductible_1) ded1 = y.tc_val;
							if (y.tc_id == deductible_2) ded2 = y.tc_val;
							if (y.tc_id == deductible_3) ded3 = y.tc_val;
						}
						// Function 19: Applies a min and max deductible on deductible % loss
						OASIS_FLOAT loss = 0;
						if (x.loss * ded1 > ded3) {
							loss = x.loss - ded3;
							if (loss < 0) loss = 0;
							x.retained_loss = x.retained_loss + (x.loss - loss);
						}
						if (x.loss * ded1 < ded2) {
							loss = x.loss - ded2;
							if (loss < 0) loss = 0;
							x.retained_loss = x.retained_loss + (x.loss - loss);
						}
						else {
							loss = x.loss - (x.loss * ded1);
							x.retained_loss = x.retained_loss + (x.loss - loss);
						}
						if (layer >1)	x.net_loss = x.net_loss + (x.previous_layer_retained_loss - loss);
						else x.net_loss = x.retained_loss;
						x.loss = loss;
					}
					break;
					case 20:
					{
						OASIS_FLOAT ded = 0;
						for (auto y : profile.tc_vec) {
							if (y.tc_id == deductible_1) ded = y.tc_val;
						}
						//Function20 = IIf(Loss > Ded, 0, Loss)
						OASIS_FLOAT loss = x.loss;
						if (loss > ded) loss = 0;
						else loss = loss;
						x.retained_loss = x.retained_loss + (x.loss - loss);
						if (layer >1)	x.net_loss = x.net_loss + (x.previous_layer_retained_loss - loss);
						else x.net_loss = x.retained_loss;
						x.loss = loss;
					}
					break;
					case 21:
					{
						OASIS_FLOAT attachment = 0;
						OASIS_FLOAT share1 = 0;
						OASIS_FLOAT limit = 0;
						for (auto y : profile.tc_vec) {
							if (y.tc_id == share_1) share1 = y.tc_val;
							if (y.tc_id == limit_1) limit = y.tc_val;
						}
						//Function21 =  Min (Max( (Loss * share1) - attachment,0), limit)
						OASIS_FLOAT loss = (x.loss * share1) - attachment;
						if (loss > limit) loss = limit;						
						if (loss < 0) loss = 0;
						x.retained_loss = x.retained_loss + (x.loss - loss);
						if (layer >1)	x.net_loss = x.net_loss + (x.previous_layer_retained_loss - loss);
						else x.net_loss = x.retained_loss;
						x.loss = loss;
					}
					break;
					case 22:
					{
						OASIS_FLOAT share1 = 0;
						OASIS_FLOAT share2 = 0;
						OASIS_FLOAT share3 = 0;
						OASIS_FLOAT limit = 0;
						for (auto y : profile.tc_vec) {
							if (y.tc_id == share_1) share1 = y.tc_val;
							if (y.tc_id == share_2) share2 = y.tc_val;
							if (y.tc_id == share_3) share3 = y.tc_val;
							if (y.tc_id == limit_1) limit = y.tc_val;
						}
						//Function22 =  Min (Loss * share1, limit) * share2 * share 3
						OASIS_FLOAT loss = x.loss * share1;
						if (loss > limit) loss = limit;
						loss = loss * share2 * share3;
						if (loss < 0) loss = 0;
						x.retained_loss = x.retained_loss + (x.loss - loss);
						if (layer >1)	x.net_loss = x.net_loss + (x.previous_layer_retained_loss - loss);
						else x.net_loss = x.retained_loss;
						x.loss = loss;
					}
					break;
					case 23:
					{
						OASIS_FLOAT share2 = 0;
						OASIS_FLOAT share3 = 0;
						OASIS_FLOAT limit = 0;
						for (auto y : profile.tc_vec) {
							if (y.tc_id == share_2) share2 = y.tc_val;
							if (y.tc_id == share_3) share3 = y.tc_val;
							if (y.tc_id == limit_1) limit = y.tc_val;
						}
						//Function23 =  Min(Loss, limit) * share 2 * share 3
						OASIS_FLOAT loss = x.loss;
						if (loss > limit) loss = limit;
						loss = loss * share2 * share3;
						if (loss < 0) loss = 0;
						x.retained_loss = x.retained_loss + (x.loss - loss);
						if (layer >1)	x.net_loss = x.net_loss + (x.previous_layer_retained_loss - loss);
						else x.net_loss = x.retained_loss;
						x.loss = loss;
					}
					break;
					case 24:
					{
						OASIS_FLOAT attachment = 0;
						OASIS_FLOAT share1 = 0;
						OASIS_FLOAT share2 = 0;
						OASIS_FLOAT share3 = 0;
						OASIS_FLOAT limit = 0;
						for (auto y : profile.tc_vec) {
							if (y.tc_id == attachment_1) attachment = y.tc_val;
							if (y.tc_id == share_1) share1 = y.tc_val;
							if (y.tc_id == share_2) share2 = y.tc_val;
							if (y.tc_id == share_3) share3 = y.tc_val;
							if (y.tc_id == limit_1) limit = y.tc_val;
						}
						//Function24 =  Min(Max(Loss - attachment,0),limit)*share 1 * share 2 * share3
						OASIS_FLOAT loss = x.loss - attachment;
						if (loss < 0) loss = 0;
						if (loss > limit) loss = limit;
						loss = loss * share1 * share2 * share3;						
						x.retained_loss = x.retained_loss + (x.loss - loss);
						if (layer >1)	x.net_loss = x.net_loss + (x.previous_layer_retained_loss - loss);
						else x.net_loss = x.retained_loss;
						x.loss = loss;
					}
					break;
					case 25:
					{
						OASIS_FLOAT share1 = 0;
						for (auto y : profile.tc_vec) {
							if (y.tc_id == share_1) share1 = y.tc_val;
						}
						//Function25 =  Loss * share1
						OASIS_FLOAT loss = x.loss * share1;
						x.retained_loss = x.retained_loss + (x.loss - loss);
						if (layer >1)	x.net_loss = x.net_loss + (x.previous_layer_retained_loss - loss);
						else x.net_loss = x.retained_loss;
						x.loss = loss;
					}
					break;
					default:
					{
						fprintf(stderr, "Unknown calc rule %d\n", profile.calcrule_id);
					}
				}
			}
		}
	}
}
