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
#include <stdio.h>

void add_tc(unsigned char tc_id, OASIS_FLOAT tc_val, std::vector<tc_rec> &tc_vec)
{
	if (tc_val > -1) {
		tc_rec t;
		t.tc_id = tc_id;
		t.tc_val = tc_val;
		tc_vec.push_back(t);
	}
}


void applycalcrule(const profile_rec_new &profile,LossRec &x,int layer)
{
	switch (profile.calcrule_id) {
		case 1:	// insurance only
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
			x.effective_deductible = x.effective_deductible + (x.loss - loss);
			if (loss > lim) {
				x.over_limit = loss - lim;
				loss = lim;
			}
			x.under_limit = lim - loss;
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
			x.effective_deductible = x.effective_deductible + (x.loss - loss);
			if (loss > (att + lim)) loss = lim;
			else loss = loss - att;
			if (loss < 0) loss = 0;
			loss = loss * share;
			//x.retained_loss = x.retained_loss + (x.loss - loss);
			OASIS_FLOAT net_loss = 0;
			if (layer > 1)	net_loss = x.previous_layer_retained_loss - loss;
			else net_loss = x.retained_loss + (x.loss - loss);
			x.retained_loss = net_loss;
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
			x.effective_deductible = x.effective_deductible + (x.loss - loss);

			if (loss > lim) {
				x.over_limit = loss - lim;
				loss = lim;
			}
			x.under_limit = lim - loss;
			//x.retained_loss = x.retained_loss + (x.loss - loss);
			OASIS_FLOAT net_loss = 0;
			if (layer > 1)	net_loss = x.previous_layer_retained_loss - loss;
			else net_loss = x.retained_loss + (x.loss - loss);
			x.retained_loss = net_loss;
			x.loss = loss;
		}
		break;
		case 4:	// insurance only
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
			x.effective_deductible = x.effective_deductible + (x.loss - loss);
			if (loss > lim) {
				x.over_limit = loss - lim;
				loss = lim;
			}
			x.under_limit = lim - loss;
			x.loss = loss;
		}
		break;
		case 5: // insurance only
		{
			OASIS_FLOAT ded = 0;
			OASIS_FLOAT lim = 0;
			for (auto y : profile.tc_vec) {
				if (y.tc_id == deductible_1) ded = y.tc_val;
				if (y.tc_id == limit_1) lim = y.tc_val;
			}
			//Function5 = Loss * (Lim - Ded)
			OASIS_FLOAT loss = x.loss - (x.loss * ded);
			x.effective_deductible = x.effective_deductible + (x.loss - loss);
			if (loss > (lim * x.loss)) {
				x.over_limit = loss - (lim * x.loss);
				loss = lim * x.loss;
			}
			x.under_limit = lim - loss;
			x.loss = loss;
		}
		break;
		case 6: // insurance only
		{
			OASIS_FLOAT ded = 0;
			for (auto y : profile.tc_vec) {
				if (y.tc_id == deductible_1) ded = y.tc_val;
			}
			//Function6 = IIf(Loss < Ded * TIV, 0, Loss -(Ded * TIV))
			OASIS_FLOAT loss = x.loss - (ded * x.accumulated_tiv);
			if (loss < 0) loss = 0;
			x.effective_deductible = x.effective_deductible + (x.loss - loss);
			x.loss = loss;
		}
		break;
		case 7: // insurance only
		{
			OASIS_FLOAT ded = 0;
			OASIS_FLOAT lim = 0;
			for (auto y : profile.tc_vec) {
				if (y.tc_id == deductible_3) ded = y.tc_val;
				if (y.tc_id == limit_1) lim = y.tc_val;
			}
			OASIS_FLOAT loss = 0;
			OASIS_FLOAT loss_delta = 0;
			if (x.effective_deductible > ded) { //If effective deductible is more than the maximum, deductible will be reduced to the maximum
				loss_delta = x.effective_deductible - ded; // loss to increase by the loss_delta
				if (x.over_limit + x.under_limit > 0) { // if there are prior level limits to reapply
					if (loss_delta > x.under_limit) { // if loss will increase beyond limit
						loss = x.loss + x.under_limit;	//truncate the loss increase at the limit
						x.effective_deductible = ded; //update the effective deductible
					}
					else {
						loss = x.loss + loss_delta;
						x.effective_deductible = x.effective_deductible - loss_delta;
					}
				}  
				else {
					loss = x.loss + loss_delta;
					x.effective_deductible = x.effective_deductible - loss_delta; //update the effective deductible to carry forward
				}
			}
			else {
				loss = x.loss; //no change to loss if effective deductible is less than the max ded.
			}

			if (loss > lim) {
				x.over_limit = loss - lim;
				loss = lim;
			}
			else x.over_limit = 0;
			x.under_limit = lim - loss;
			x.loss = loss;
			
		}
		break;
		case 8:	// insurance only
		{
			OASIS_FLOAT ded = 0;
			OASIS_FLOAT lim = 0;
			for (auto y : profile.tc_vec) {
				if (y.tc_id == deductible_2) ded = y.tc_val;
				if (y.tc_id == limit_1) lim = y.tc_val;
			}
			OASIS_FLOAT loss = 0;
			OASIS_FLOAT loss_delta = 0;
			if (x.effective_deductible < ded) { // If effective deductible is less than the minimum, deductible will be increased to the minimum
				loss_delta = x.effective_deductible - ded; // negative loss change
				if (x.over_limit + x.under_limit > 0) { // if there are prior level limits to reapply
					if (x.under_limit == 0) { // If carried loss is at a prior level limit
						if (-loss_delta > x.over_limit) { // if the loss decrease will take the loss back through the prior level limits
							loss = x.loss + x.over_limit + x.effective_deductible - ded; //let the loss decrease by the difference between the overlimit and the loss delta
							x.effective_deductible = ded;
						}
						else loss = x.loss; // no change to the loss because the adjusted loss is still overlimit
						x.effective_deductible = ded;//update the deductible to carry forward
					}
					else {
						loss = x.loss + loss_delta; // loss decreases by the full difference between effective deductible and min deductible
					}
				}
				else {
					loss = x.loss + loss_delta;
					if (loss < 0) loss = 0; //loss can't go negative
					x.effective_deductible = x.effective_deductible + (x.loss - loss); //update the effective deductible to carry forward
				}
			}
			else {
				loss = x.loss; //no change to loss if effective deductible is more than the min ded.
			}
			
			if (loss > lim) {
				x.over_limit = loss - lim;
				loss = lim;
			}
			x.under_limit = lim - loss;
			x.loss = loss;
		
		}
		break;
		case 9: // insurance only
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
			x.effective_deductible = x.effective_deductible + (x.loss - loss);
			if (loss > lim) {
				x.over_limit = loss - lim;
				loss = lim;
			}
			x.under_limit = lim - loss;
			x.loss = loss;
		}
		break;
		case 10: // insurance only
		{
			OASIS_FLOAT ded = 0;
			for (auto y : profile.tc_vec) {
				if (y.tc_id == deductible_3) ded = y.tc_val;
			}
			// Function 10: Applies a cap on retained loss (maximum deductible)
			OASIS_FLOAT loss = 0;
			OASIS_FLOAT loss_delta = 0;
			if (x.effective_deductible > ded) { //If effective deductible is more than the maximum, deductible will be reduced to the maximum
				loss_delta = x.effective_deductible - ded; // loss to increase by the loss_delta
				if (x.over_limit + x.under_limit > 0) { //if there are prior level limits to reapply
					if (loss_delta > x.under_limit) { // if loss will increase beyond limit
						loss = x.loss + x.under_limit;	//truncate the loss increase at the limit
						x.over_limit = x.over_limit + (loss_delta - x.under_limit); //update the overlimit
						x.under_limit = 0; //update the underlimit
						x.effective_deductible = ded; //update the deductible to carry forward			
					}
					else {
						loss = x.loss + loss_delta; // else increase by the full loss delta
						x.under_limit = x.under_limit - loss_delta; //update the underlimit
						x.effective_deductible = x.effective_deductible + (x.loss - loss);
					}
				}
				else {
					loss = x.loss + loss_delta; // else increase by the full loss delta
					x.effective_deductible = x.effective_deductible + (x.loss - loss); //update the deductible to carry forward
				}
			}
			else {
				loss = x.loss; //no change to loss if carried deductible is less than the max ded.
			}
			x.loss = loss;
		}
		break;
		case 11: 
		{
			OASIS_FLOAT ded = 0;
			for (auto y : profile.tc_vec) {
				if (y.tc_id == deductible_2) ded = y.tc_val;
			}
			OASIS_FLOAT loss = 0;
			OASIS_FLOAT loss_delta = 0;
			if (x.effective_deductible < ded) { // If effective deductible is less than the minimum, deductible will be increased to the minimum
				loss_delta = x.effective_deductible - ded; // negative loss change
				if (x.over_limit + x.under_limit > 0) { // If there are prior level limits to reapply
					if (x.under_limit == 0) { // If carried loss is at a prior level limit
						if (-loss_delta > x.over_limit) { // if the loss decrease will take the loss back through the prior level limits
							loss = x.loss + x.over_limit + x.effective_deductible - ded; //let the loss decrease by the difference between the overlimit and the loss delta
							x.over_limit = 0; //update the overlimit
						}
						else {
							loss = x.loss; // no change to the loss because the adjusted loss is still overlimit
							x.over_limit = x.over_limit + loss_delta; // reduce the overlimit by the loss delta
						}
					}
					else {
						loss = x.loss + loss_delta; // loss decreases by the full difference between effective deductible and min deductible
						if (loss < 0) loss = 0; //loss can't go negative
						x.under_limit = x.under_limit + (x.loss - loss); // underlimit increases by the change in loss
					}
				}

				else {
					loss = x.loss + loss_delta; // loss decreases by the full difference between effective deductible and min deductible
				}
				if (loss < 0) loss = 0; //loss can't go negative
				x.effective_deductible = x.effective_deductible + (x.loss - loss); //update the deductible to carry forward
			}
			else {
				loss = x.loss; //no change to loss if carried deductible is more than the min ded.
			}
			x.loss = loss;
		}
		break;
		case 12:
		{
			if (x.loss > 0) {
				for (auto &z : profile.tc_vec) {
					if (z.tc_id == deductible_1) {
						OASIS_FLOAT loss = x.loss - z.tc_val;
						if (loss < 0) loss = 0;
						x.effective_deductible = x.effective_deductible + (x.loss - loss);
						x.retained_loss = x.retained_loss + (x.loss - loss);
						x.loss = loss;
						break;
					}
				}
			}
		}
		break;
		case 13: // insurance only
		{
			OASIS_FLOAT ded2 = 0;
			OASIS_FLOAT ded3 = 0;
			for (auto y : profile.tc_vec) {
				if (y.tc_id == deductible_2) ded2 = y.tc_val;
				if (y.tc_id == deductible_3) ded3 = y.tc_val;
			}
			OASIS_FLOAT loss = 0;
			OASIS_FLOAT loss_delta = 0;

			if (x.effective_deductible > ded3) { //If effective deductible is more than the maximum, deductible will be reduced to the maximum
				loss_delta = x.effective_deductible - ded3; // loss to increase by the loss_delta
				if (x.over_limit + x.under_limit > 0) { //if there are prior level limits to reapply
					if (loss_delta > x.under_limit) { // if loss will increase beyond limit
						loss = x.loss + x.under_limit;	//truncate the loss increase at the limit
						x.over_limit = x.over_limit + (loss_delta - x.under_limit); //update the overlimit
						x.under_limit = 0; //update the underlimit
						x.effective_deductible = ded3;
					}
					else {
						loss = x.loss + loss_delta; // else increase by the full loss delta
						x.under_limit = x.under_limit - loss_delta; //update the underlimit
						x.effective_deductible = x.effective_deductible + (x.loss - loss); //update the deductible to carry forward
					}
				}
				else {
					loss = x.loss + loss_delta; // else increase by the full loss delta
					x.effective_deductible = x.effective_deductible + (x.loss - loss); //update the deductible to carry forward
				}
			}
			else {
				if (x.effective_deductible < ded2) { // If effective deductible is less than the minimum, deductible will be increased to the minimum
					loss_delta = x.effective_deductible - ded2; // negative loss change
					if (x.over_limit + x.under_limit > 0) { // If there are prior level limits to reapply
						if (x.under_limit == 0) { // If carried loss is at a prior level limit
							if (-loss_delta > x.over_limit) { // if the loss decrease will take the loss back through the prior level limits
								loss = x.loss + x.over_limit + x.effective_deductible - ded2; //let the loss decrease by the difference between the overlimit and the loss delta
								x.over_limit = 0; //update the overlimit
							}
							else {
								loss = x.loss; // no change to the loss because the adjusted loss is still overlimit
								x.over_limit = x.over_limit + loss_delta; // reduce the overlimit by the loss delta
							}
						}
						else {
							loss = x.loss + loss_delta; // loss decreases by the full difference between effective deductible and min deductible
							if (loss < 0) loss = 0; //loss can't go negative
							x.under_limit = x.under_limit + (x.loss - loss); // underlimit increases by the change in loss
						}
					}
					else {
						loss = x.loss + loss_delta; // loss decreases by the full difference between effective deductible and min deductible
					}
					if (loss < 0) loss = 0; //loss can't go negative
					x.effective_deductible = x.effective_deductible + (x.loss - loss); //update the deductible to carry forward
				}
				else {
					loss = x.loss;
					// effective deductible / retained loss stays the same
				}
			}
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
			if (loss > lim) {
				x.over_limit = loss - lim;
				loss = lim;
			}
			x.under_limit = lim - loss;
			//x.retained_loss = x.retained_loss + (x.loss - loss);
			OASIS_FLOAT net_loss = 0;
			if (layer > 1)	net_loss = x.previous_layer_retained_loss - loss;
			else net_loss = x.retained_loss + (x.loss - loss);
			x.retained_loss = net_loss;
			x.loss = loss;
		}
		break;
		case 15: // insurance only
		{
			OASIS_FLOAT lim = 0;
			for (auto y : profile.tc_vec) {
				if (y.tc_id == limit_1) lim = y.tc_val;
			}
			//Function15 =  Loss * lim
			OASIS_FLOAT loss = x.loss;
			loss = loss * lim;
			if (x.loss > loss) {
				x.over_limit = x.loss - loss;
				}
			x.under_limit = lim - loss;			
			x.loss = loss;
		}
		break;
		case 16: // insurance only
		{
			OASIS_FLOAT ded = 0;
			for (auto y : profile.tc_vec) {
				if (y.tc_id == deductible_1) ded = y.tc_val;
			}
			//Function16 =  Loss - (loss * ded)
			OASIS_FLOAT loss = x.loss;
			loss = loss - (loss * ded);
			if (loss < 0) loss = 0;
			x.effective_deductible = x.effective_deductible + (x.loss - loss);
			x.loss = loss;
		}
		break;
		case 17: // insurance only
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
			x.effective_deductible = x.effective_deductible + (x.loss - loss);
			if (loss > (att + lim)) loss = lim;
			else loss = loss - att;
			if (loss < 0) loss = 0;
			loss = loss * share;
			x.loss = loss;
		}
		break;
		case 18: // insurance only
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
			x.effective_deductible = x.effective_deductible + (x.loss - loss);
			if (loss > (att + lim)) loss = lim;
			else loss = loss - att;
			if (loss < 0) loss = 0;
			loss = loss * share;
			x.loss = loss;
		}
		break;
		case 19:// insurance only
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
			OASIS_FLOAT loss_delta = 0;

			if ((x.loss * ded1) + x.effective_deductible > ded3) {
				loss_delta =  x.effective_deductible - ded3; // loss to increase by the loss_delta
				if (x.over_limit + x.under_limit > 0) { //if there are prior level limits to reapply
					if (loss_delta > x.under_limit) { // if loss will increase beyond limit
						loss = x.loss + x.under_limit;	//truncate the loss increase at the limit
						x.over_limit = x.over_limit + (loss_delta - x.under_limit); //update the overlimit
						x.under_limit = 0; //update the underlimit
						x.effective_deductible = ded3; //update the deductible to carry forward
					}
					else {
						loss = x.loss + loss_delta; // else increase by the full loss delta
						x.under_limit = x.under_limit - loss_delta; //update the underlimit
						x.effective_deductible = x.effective_deductible + (x.loss - loss);
					}
				}
				else {
					loss = x.loss + loss_delta; // else increase by the full loss delta
					x.effective_deductible = x.effective_deductible + (x.loss - loss); //update the deductible to carry forward
				}
			}
			else {
				if ((x.loss * ded1) + x.effective_deductible < ded2) {
					loss_delta =  x.effective_deductible - ded2; // negative loss change
					if (x.over_limit + x.under_limit > 0) { // If there are prior level limits
						if (x.under_limit == 0) { // If carried loss is at a prior level limit
							if (-loss_delta > x.over_limit) { // if the loss decrease will take the loss back through the prior level limits
								loss = x.loss + x.over_limit + x.effective_deductible - ded2; //let the loss decrease by the difference between the overlimit and the loss delta
								x.over_limit = 0; //update the overlimit
							}
							else {
								loss = x.loss; // no change to the loss because the adjusted loss is still overlimit
								x.over_limit = x.over_limit + loss_delta; // reduce the overlimit by the loss delta
							}
						}
						else {
							loss = x.loss + loss_delta; // loss decreases by the full difference between effective deductible and min deductible
							if (loss < 0) loss = 0; //loss can't go negative
							x.under_limit = x.under_limit + (x.loss - loss); // underlimit increases by the change in loss
						}
					}
					else {
						loss = x.loss + loss_delta; // loss decreases by the full difference between effective deductible and min deductible
					}
					if (loss < 0) loss = 0; //loss can't go negative
					x.effective_deductible = x.effective_deductible + (x.loss - loss); //update the deductible to carry forward
				}
				else {
					loss = x.loss - (x.loss * ded1);
					if (loss < 0) loss = 0;
					x.effective_deductible = x.effective_deductible + (x.loss - loss);
					//x.retained_loss = x.retained_loss + (x.loss - loss);		
				}
			}			
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
			//x.retained_loss = x.retained_loss + (x.loss - loss);
			OASIS_FLOAT net_loss = 0;
			if (layer > 1)	net_loss = x.previous_layer_retained_loss - loss;
			else net_loss = x.retained_loss + (x.loss - loss);
			x.retained_loss = net_loss;
			x.loss = loss;
		}
		break;
		case 21:
			//  minimum and maximum applied to prior level effective deductible plus % tiv deductible. to do
		{
			OASIS_FLOAT ded1 = 0;
			OASIS_FLOAT ded2 = 0;
			OASIS_FLOAT ded3 = 0;
			for (auto y : profile.tc_vec) {
				if (y.tc_id == deductible_1) ded1 = y.tc_val;
				if (y.tc_id == deductible_2) ded2 = y.tc_val;
				if (y.tc_id == deductible_3) ded3 = y.tc_val;
			}
			OASIS_FLOAT loss = 0;
			OASIS_FLOAT effective_ded = 0;
			OASIS_FLOAT loss_delta = 0;

			effective_ded = x.accumulated_tiv * ded1;
			if (effective_ded > x.loss) effective_ded = x.loss;
			if ((effective_ded + x.effective_deductible) > ded3) {
				loss_delta =  x.effective_deductible - ded3;
				if (x.over_limit + x.under_limit > 0) { //if there are prior level limits to reapply
					if (loss_delta > x.under_limit) { // if loss will increase beyond limit
						loss = x.loss + x.under_limit;	//truncate the loss increase at the limit
						x.over_limit = x.over_limit + (loss_delta - x.under_limit); //update the overlimit
						x.under_limit = 0; //update the underlimit
						x.effective_deductible = ded3;//update the deductible to carry forward
					}
					else {
						loss = x.loss + loss_delta; // else increase by the full loss delta
						x.under_limit = x.under_limit - loss_delta; //update the underlimit
						x.effective_deductible = x.effective_deductible + (x.loss - loss);//update the deductible to carry forward
					}
				}
				else {
					loss = x.loss + loss_delta; // else increase by the full loss delta
					x.effective_deductible = x.effective_deductible + (x.loss - loss); //update the deductible to carry forward
				}
			}
			else {
				if ((effective_ded + x.effective_deductible) < ded2) {
					loss_delta = x.effective_deductible - ded2;
					if (x.over_limit + x.under_limit > 0) { // If there are prior level limits to reapply
						if (x.under_limit == 0) { // If carried loss is at a prior level limit
							if (-loss_delta > x.over_limit) { // if the loss decrease will take the loss back through the prior level limits
								loss = x.loss + x.over_limit + x.effective_deductible - ded2; //let the loss decrease by the difference between the overlimit and the loss delta
								x.over_limit = 0; //update the overlimit
							}
							else {
								loss = x.loss; // no change to the loss because the adjusted loss is still overlimit
								x.over_limit = x.over_limit + loss_delta; // reduce the overlimit by the loss delta
							}
						}
						else {
							loss = x.loss + loss_delta; // loss decreases by the full difference between effective deductible and min deductible
							if (loss < 0) loss = 0; //loss can't go negative
							x.under_limit = x.under_limit + (x.loss - loss); // underlimit increases by the change in loss
						}
					}
					else {
						loss = x.loss + loss_delta; // loss decreases by the full difference between effective deductible and min deductible
					}
					if (loss < 0) loss = 0; //loss can't go negative
					x.effective_deductible = x.effective_deductible + (x.loss - loss); //update the deductible to carry forward
				}
				else {
					loss = x.loss - effective_ded;
					if (loss < 0) loss = 0;
					x.effective_deductible = x.effective_deductible + (x.loss - loss);
					//x.retained_loss = x.retained_loss + (x.loss - loss);		
				}
			}
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
			//x.retained_loss = x.retained_loss + (x.loss - loss);
			OASIS_FLOAT net_loss = 0;
			if (layer > 1)	net_loss = x.previous_layer_retained_loss - loss;
			else net_loss = x.retained_loss + (x.loss - loss);
			x.retained_loss = net_loss;
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
			//x.retained_loss = x.retained_loss + (x.loss - loss);
			OASIS_FLOAT net_loss = 0;
			if (layer > 1)	net_loss = x.previous_layer_retained_loss - loss;
			else net_loss = x.retained_loss + (x.loss - loss);
			x.retained_loss = net_loss;
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
			//Function24 =  Min(Max((Loss *share1) - attachment,0),limit)* share2 * share3
			OASIS_FLOAT loss = (x.loss * share1) - attachment;
			if (loss < 0) loss = 0;
			if (loss > limit) loss = limit;
			loss = loss * share2 * share3;
			//x.retained_loss = x.retained_loss + (x.loss - loss);
			OASIS_FLOAT net_loss = 0;
			if (layer > 1)	net_loss = x.previous_layer_retained_loss - loss;
			else net_loss = x.retained_loss + (x.loss - loss);
			x.retained_loss = net_loss;
			x.loss = loss;
		}
		break;
		case 25:
		{
			OASIS_FLOAT share1 = 0;
			OASIS_FLOAT share2 = 0;
			OASIS_FLOAT share3 = 0;
			for (auto y : profile.tc_vec) {
				if (y.tc_id == share_1) share1 = y.tc_val;
				if (y.tc_id == share_2) share2 = y.tc_val;
				if (y.tc_id == share_3) share3 = y.tc_val;
			}
			//Function25 =  Loss * share1
			OASIS_FLOAT loss = x.loss * share1 * share2 * share3;
			//x.retained_loss = x.retained_loss + (x.loss - loss);
			OASIS_FLOAT net_loss = 0;
			if (layer > 1)	net_loss = x.previous_layer_retained_loss - loss;
			else net_loss = x.retained_loss + (x.loss - loss);
			x.retained_loss = net_loss;
			x.loss = loss;
		}
		break;
		case 26:// insurance only
		{
			OASIS_FLOAT ded1 = 0;
			OASIS_FLOAT ded2 = 0;
			OASIS_FLOAT ded3 = 0;
			for (auto y : profile.tc_vec) {
				if (y.tc_id == deductible_1) ded1 = y.tc_val;
				if (y.tc_id == deductible_2) ded2 = y.tc_val;
				if (y.tc_id == deductible_3) ded3 = y.tc_val;
			}
			OASIS_FLOAT loss = 0;
			OASIS_FLOAT loss_delta = 0;
			// Applies a min and max ded on effective deductible plus a deductible as an amount.
			if ((ded1 + x.effective_deductible) > ded3) { //If carried + ded > max ded
				loss_delta = x.effective_deductible - ded3;
				if (x.over_limit + x.under_limit > 0) { //if there are prior level limits to reapply
					if (loss_delta > x.under_limit) { // if loss will increase beyond limit
						loss = x.loss + x.under_limit;	//truncate the loss increase at the limit
						x.over_limit = x.over_limit + (loss_delta - x.under_limit); //update the overlimit
						x.under_limit = 0; //update the underlimit
						x.effective_deductible = ded3;//update the deductible to carry forward
					}
					else {
						loss = x.loss + loss_delta; // else increase by the full loss delta
						x.under_limit = x.under_limit - loss_delta; //update the underlimit
						x.effective_deductible = x.effective_deductible + (x.loss - loss);//update the deductible to carry forward
					}
				}
				else {
					loss = x.loss + loss_delta; // else increase by the full loss delta
					x.effective_deductible = x.effective_deductible + (x.loss - loss); //update the deductible to carry forward
				}
			}
			else {
				if ((ded1 + x.effective_deductible) < ded2) { //If carried + ded < min ded
					loss_delta = x.effective_deductible - ded2;
					if (x.over_limit + x.under_limit > 0) { // If there are prior level limits to reapply
						if (x.under_limit == 0) { // If carried loss is at a prior level limit
							if (-loss_delta > x.over_limit) { // if the loss decrease will take the loss back through the prior level limits
								loss = x.loss + x.over_limit + x.effective_deductible - ded2; //let the loss decrease by the difference between the overlimit and the loss delta
								x.over_limit = 0; //update the overlimit
							}
							else {
								loss = x.loss; // no change to the loss because the adjusted loss is still overlimit
								x.over_limit = x.over_limit + loss_delta; // reduce the overlimit by the loss delta
							}
						}
						else {
							loss = x.loss + loss_delta; // loss decreases by the full difference between effective deductible and min deductible
							if (loss < 0) loss = 0; //loss can't go negative
							x.under_limit = x.under_limit + (x.loss - loss); // underlimit increases by the change in loss
						}
					}
					else {
						loss = x.loss + loss_delta; // loss decreases by the full difference between effective deductible and min deductible
					}
					if (loss < 0) loss = 0; //loss can't go negative
					x.effective_deductible = x.effective_deductible + (x.loss - loss); //update the deductible to carry forward
				}
				else { // min ded < carried ded + ded < max ded
					loss = x.loss - ded1;
					if (loss < 0) loss = 0;
					x.effective_deductible = x.effective_deductible + (x.loss - loss);
					//x.retained_loss = x.retained_loss + (x.loss - loss);		
				}
			}
			x.loss = loss;
		}
		default:
		{
			fprintf(stderr, "Unknown calc rule %d\n", profile.calcrule_id);
		}
	}
}

void fmcalc::dofmcalc(std::vector <LossRec> &agg_vec, int layer)
{
	for (LossRec &x : agg_vec) {
		if (x.agg_id == 0) break;
		if (x.agg_id > 0) {
			if (x.loss > 0 || x.retained_loss > 0) {
				if (x.policytc_id > 0) {
					const profile_rec_new &profile = profile_vec_new_[x.policytc_id];					
					applycalcrule(profile, x, layer);
				}
			}
		}
	}
}

void fmcalc::init_profile_rec(fm_profile &f)
{
	profile_rec_new p;
	p.calcrule_id = f.calcrule_id;
	switch (p.calcrule_id) {
		case 1:
			add_tc(deductible_1, f.deductible1, p.tc_vec);
			add_tc(limit_1, f.limit, p.tc_vec);
			break;
		case 2:
			add_tc(deductible_1, f.deductible1, p.tc_vec);
			add_tc(limit_1, f.limit, p.tc_vec);
			add_tc(share_1, f.share1, p.tc_vec);
			add_tc(attachment_1, f.attachment, p.tc_vec);
			break;
		case 3:
			add_tc(deductible_1, f.deductible1, p.tc_vec);
			add_tc(limit_1, f.limit, p.tc_vec);
			break;
		case 4:
			add_tc(deductible_1, f.deductible1, p.tc_vec);
			add_tc(limit_1, f.limit, p.tc_vec);
			break;
		case 5:
			add_tc(deductible_1, f.deductible1, p.tc_vec);
			add_tc(limit_1, f.limit, p.tc_vec);
			break;
		case 6:
			add_tc(deductible_1, f.deductible1, p.tc_vec);
			break;
		case 9:
			add_tc(deductible_1, f.deductible1, p.tc_vec);
			add_tc(limit_1, f.limit, p.tc_vec);
			break;
		case 10:
			add_tc(deductible_3, f.deductible3, p.tc_vec);
			break;
		case 11:
			add_tc(deductible_2, f.deductible2, p.tc_vec);
			break;
		case 12:
			add_tc(deductible_1, f.deductible1, p.tc_vec);
			break;
		case 14:
			add_tc(limit_1, f.limit, p.tc_vec);
			break;
		case 15:
			add_tc(limit_1, f.limit, p.tc_vec);
			break;
		case 16:
			add_tc(deductible_1, f.deductible1, p.tc_vec);
			break;
		case 17:
			add_tc(deductible_1, f.deductible1, p.tc_vec);
			add_tc(limit_1, f.limit, p.tc_vec);
			add_tc(share_1, f.share1, p.tc_vec);
			add_tc(attachment_1, f.attachment, p.tc_vec);
			break;
		case 18:
			add_tc(deductible_1, f.deductible1, p.tc_vec);
			add_tc(limit_1, f.limit, p.tc_vec);
			add_tc(share_1, f.share1, p.tc_vec);
			add_tc(attachment_1, f.attachment, p.tc_vec);
			break;
		case 19:
			add_tc(deductible_1, f.deductible1, p.tc_vec);
			add_tc(deductible_2, f.deductible2, p.tc_vec);
			add_tc(deductible_3, f.deductible3, p.tc_vec);
			break;
		case 20:
			add_tc(deductible_1, f.deductible1, p.tc_vec);
			break;
		case 21:
			add_tc(deductible_1, f.deductible1, p.tc_vec);
			add_tc(deductible_2, f.deductible2, p.tc_vec);
			add_tc(deductible_3, f.deductible3, p.tc_vec);
			break;
		case 23:
			add_tc(share_2, f.share2, p.tc_vec);
			add_tc(share_3, f.share3, p.tc_vec);
			add_tc(limit_1, f.limit, p.tc_vec);
			break;
		case 24:
			add_tc(attachment_1, f.attachment, p.tc_vec);
			add_tc(share_1, f.share1, p.tc_vec);
			add_tc(share_2, f.share2, p.tc_vec);
			add_tc(share_3, f.share3, p.tc_vec);
			add_tc(limit_1, f.limit, p.tc_vec);
			break;
		case 25:
			add_tc(share_1, f.share1, p.tc_vec);
			add_tc(share_2, f.share2, p.tc_vec);
			add_tc(share_3, f.share3, p.tc_vec);
			break;
		default:
		{
			//fprintf(stderr, "Found calrule %d\n", p.calcrule_id);
			add_tc(deductible_1, f.deductible1, p.tc_vec);
			add_tc(deductible_2, f.deductible2, p.tc_vec);
			add_tc(deductible_3, f.deductible3, p.tc_vec);
			add_tc(attachment_1, f.attachment, p.tc_vec);
			add_tc(limit_1, f.limit, p.tc_vec);
			add_tc(share_1, f.share1, p.tc_vec);
			add_tc(share_2, f.share2, p.tc_vec);
			add_tc(share_3, f.share3, p.tc_vec);
		}

	}
	if (profile_vec_new_.size() < f.profile_id + 1) {
		profile_vec_new_.resize(f.profile_id + 1);
	}
	profile_vec_new_[f.profile_id] = p;
}

void fmcalc::init_profile()
{
	FILE *fin = NULL;
	std::string file = FMPROFILE_FILE_NEW;
	if (inputpath_.length() > 0) {
		file = inputpath_ + file.substr(5);
	}
	fin = fopen(file.c_str(), "rb");
	if (fin == NULL) {
		fprintf(stderr, "%s: cannot open %s\n", __func__, file.c_str());
		exit(EXIT_FAILURE);
	}
	fm_profile f;
	int i = fread(&f, sizeof(f), 1, fin);
	while (i != 0) {		
		init_profile_rec(f);
		if (noop_profile_id < f.profile_id) noop_profile_id = f.profile_id;
		i = fread(&f, sizeof(f), 1, fin);
	}
	noop_profile_id++;
	fm_profile d;	// dummy
	d.profile_id = noop_profile_id;
	d.calcrule_id = 14;
	init_profile_rec(d);
	fclose(fin);
}
