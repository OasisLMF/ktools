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

void applycalcrule_stepped(const profile_rec_new& profile, LossRec& x, int layer,bool isLast)
{
	switch (profile.calcrule_id) {
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
	case 12:
	{
		if (x.loss > 0) {
			for (auto& z : profile.tc_vec) {
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
	case 27:
	{
		OASIS_FLOAT deductible1 = 0;
		OASIS_FLOAT tstart = 0;
		OASIS_FLOAT tend = 0;
		OASIS_FLOAT payout = 0;
		OASIS_FLOAT scale1 = 0;
		OASIS_FLOAT scale2 = 0;
		OASIS_FLOAT limit1 = 0;
		OASIS_FLOAT limit2 = 0;
		for (auto y : profile.tc_vec) {
			if (y.tc_id == deductible_1) deductible1 = y.tc_val;
			if (y.tc_id == trigger_start) tstart = y.tc_val;
			if (y.tc_id == trigger_end) tend = y.tc_val;
			if (y.tc_id == payout_start) payout = y.tc_val;
			if (y.tc_id == scale_1) scale1 = y.tc_val;
			if (y.tc_id == scale_2) scale2 = y.tc_val;
			if (y.tc_id == limit_1) limit1 = y.tc_val;
			if (y.tc_id == limit_2) limit2 = y.tc_val;

		}
		// Step policy: single step with % loss tiv payout and limit, optional deductible, extra expense payout with limit amount, and a gross up factor for debris removal
		OASIS_FLOAT loss = 0;
		OASIS_FLOAT condloss = 0;
		loss = x.loss / x.accumulated_tiv;
		if (tend == 1) { // if the upper threshold is 100% then include loss = tend in the conditional calculation
			if (loss <= tend) {
				if (loss >= tstart) {
					loss = (payout * x.accumulated_tiv) - deductible1; //calculate primary payout
					if (loss < 0) loss = 0;
					if (loss > limit1) loss = limit1;
					condloss = loss * scale2; //calculate conditional payout based on primary coverage payout (extra expenses)
					if (condloss > limit2) condloss = limit2; //limit conditional payout
					loss = loss + condloss; // main coverage + extra expense payout
					loss = loss * (1 + scale1); // gross up for debris removal
				}
				else loss = 0;
			}
			else loss = 0;
		}
		else { // if the upper threshold is not 100% then do not include loss = tend in the conditional calculation
			if (loss < tend) {
				if (loss >= tstart) {
					loss = (payout * x.accumulated_tiv) - deductible1; //calculate primary payout
					if (loss < 0) loss = 0;
					if (loss > limit1) loss = limit1;
					condloss = loss * scale2; //calculate conditional payout based on primary coverage payout (extra expenses)
					if (condloss > limit2) condloss = limit2; //limit conditional payout
					loss = loss + condloss; // main coverage + extra expense payout
					loss = loss * (1 + scale1); // gross up for debris removal
				}
				else loss = 0;
			}
			else loss = 0;
		}

		if (profile.step_id == 1) {
			x.step_loss = loss;
		}
		else {
			x.step_loss = x.step_loss + loss;
		}
		if (isLast == true) {
			x.loss = x.step_loss;
		}
	}
	break;
		case 28:
		{		
			OASIS_FLOAT deductible1 = 0;
			OASIS_FLOAT tstart = 0;
			OASIS_FLOAT tend = 0;
			OASIS_FLOAT payout = 0;
			OASIS_FLOAT scale1 = 0;
			OASIS_FLOAT scale2 = 0;
			OASIS_FLOAT limit2 = 0;
			

			for (auto y : profile.tc_vec) {
				if (y.tc_id == deductible_1) deductible1 = y.tc_val;
				if (y.tc_id == trigger_start) tstart = y.tc_val;
				if (y.tc_id == trigger_end) tend = y.tc_val;
				if (y.tc_id == payout_start) payout = y.tc_val;
				if (y.tc_id == scale_1) scale1 = y.tc_val;
				if (y.tc_id == scale_2) scale2 = y.tc_val;
				if (y.tc_id == limit_2) limit2 = y.tc_val;				
			}
			// Step policy: single (final) step with % loss payout, no limit, extra expense payout with limit amount, and a gross up factor for debris removal
			OASIS_FLOAT loss = 0;
			OASIS_FLOAT condloss = 0;
			loss = x.loss / x.accumulated_tiv;
			if (tend == 1) { // if the upper threshold is 100% then include loss = tend in the conditional calculation
				if (loss <= tend) {
					if (loss >= tstart) {
						loss = (payout * x.loss) - deductible1; //calculate primary payout
						if (loss < 0) loss = 0;
						if (payout == 0) { 
							condloss = x.step_loss * scale2; //special case to calculate only the conditional coverage loss (extra expenses) based on full input loss where there is no primary payout
						}
						else condloss = loss * scale2; //calculate conditional payout based on primary coverage payout (extra expenses)
						if (condloss > limit2) condloss = limit2; //limit conditional payout
						loss = loss + condloss; // main coverage + extra expense payout
						loss = loss * (1 + scale1); // gross up for debris removal
					}
					else loss = 0;
				}
				else loss = 0;
			}
			else { // if the upper threshold is not 100% then do not include loss = tend in the conditional calculation
				if (loss < tend) {
					if (loss >= tstart) {
						loss = (payout * x.loss) - deductible1; //calculate primary payout
						if (loss < 0) loss = 0;
						if (payout == 0) {
							condloss = x.step_loss * scale2; //special case to calculate only the conditional coverage loss (extra expenses) based on the step loss brought forward.
						}
						else condloss = loss * scale2; //calculate conditional payout (extra expenses)
						if (condloss > limit2) condloss = limit2; //limit conditional payout
						loss = loss + condloss; // main coverage + extra expense payout
						loss = loss * (1 + scale1); // gross up for debris removal
					}
					else loss = 0;
				}
				else loss = 0;
			}
			
			if (profile.step_id == 1) {
				x.step_loss = loss;
			}
			else {
				x.step_loss = x.step_loss + loss;
			}
			if (isLast == true) {
				x.loss = x.step_loss;
			}
		}
		break;
		case 29:
		{
			OASIS_FLOAT deductible1 = 0;
			OASIS_FLOAT tstart = 0;
			OASIS_FLOAT tend = 0;
			OASIS_FLOAT payout = 0;
			OASIS_FLOAT scale1 = 0;
			OASIS_FLOAT scale2 = 0;
			OASIS_FLOAT limit2 = 0;
			for (auto y : profile.tc_vec) {
				if (y.tc_id == deductible_1) deductible1 = y.tc_val;
				if (y.tc_id == trigger_start) tstart = y.tc_val;
				if (y.tc_id == trigger_end) tend = y.tc_val;
				if (y.tc_id == payout_start) payout = y.tc_val;
				if (y.tc_id == scale_1) scale1 = y.tc_val;
				if (y.tc_id == scale_2) scale2 = y.tc_val;
				if (y.tc_id == limit_2) limit2 = y.tc_val;

			}
			// Step policy: single step with % loss tiv payout, no limit, optional deductible, extra expense payout with limit amount, and a gross up factor for debris removal
			OASIS_FLOAT loss = 0;
			OASIS_FLOAT condloss = 0;
			loss = x.loss / x.accumulated_tiv;
			if (tend == 1) { // if the upper threshold is 100% then include loss = tend in the conditional calculation
				if (loss <= tend) {
					if (loss >= tstart) {
						loss = (payout * x.accumulated_tiv) - deductible1; //calculate primary payout
						if (loss < 0) loss = 0;
						condloss = loss * scale2; //calculate conditional payout based on primary coverage payout (extra expenses)
						if (condloss > limit2) condloss = limit2; //limit conditional payout
						loss = loss + condloss; // main coverage + extra expense payout
						loss = loss * (1 + scale1); // gross up for debris removal
					}
					else loss = 0;
				}
				else loss = 0;
			}
			else { // if the upper threshold is not 100% then do not include loss = tend in the conditional calculation
				if (loss < tend) {
					if (loss >= tstart) {
						loss = (payout * x.accumulated_tiv) - deductible1; //calculate primary payout
						if (loss < 0) loss = 0;
						condloss = loss * scale2; //calculate conditional payout based on primary coverage payout (extra expenses)
						if (condloss > limit2) condloss = limit2; //limit conditional payout
						loss = loss + condloss; // main coverage + extra expense payout
						loss = loss * (1 + scale1); // gross up for debris removal
					}
					else loss = 0;
				}
				else loss = 0;
			}

			if (profile.step_id == 1) {
				x.step_loss = loss;
			}
			else {
				x.step_loss = x.step_loss + loss;
			}
			if (isLast == true) {
				x.loss = x.step_loss;
			}
		}
		break;
		case 30:
		{
			OASIS_FLOAT deductible1 = 0;
			OASIS_FLOAT tstart = 0;
			OASIS_FLOAT tend = 0;
			OASIS_FLOAT payout = 0;
			OASIS_FLOAT scale1 = 0;
			OASIS_FLOAT scale2 = 0;
			OASIS_FLOAT limit1 = 0;
			OASIS_FLOAT limit2 = 0;
			for (auto y : profile.tc_vec) {
				if (y.tc_id == deductible_1) deductible1 = y.tc_val;
				if (y.tc_id == trigger_start) tstart = y.tc_val;
				if (y.tc_id == trigger_end) tend = y.tc_val;
				if (y.tc_id == payout_start) payout = y.tc_val;
				if (y.tc_id == scale_1) scale1 = y.tc_val;
				if (y.tc_id == scale_2) scale2 = y.tc_val;
				if (y.tc_id == limit_1) limit1 = y.tc_val;
				if (y.tc_id == limit_2) limit2 = y.tc_val;

			}
			// Step policy: single step with % limit payout, optional deductible, extra expense payout with limit amount, and a gross up factor for debris removal
			OASIS_FLOAT loss = 0;
			OASIS_FLOAT condloss = 0;
			loss = x.loss / x.accumulated_tiv;
			if (tend == 1) { // if the upper threshold is 100% then include loss = tend in the conditional calculation
				if (loss <= tend) {
					if (loss >= tstart) {
						loss = (payout * limit1) - deductible1; //calculate primary payout
						if (loss < 0) loss = 0;
						condloss = loss * scale2; //calculate conditional payout based on primary coverage payout (extra expenses)
						if (condloss > limit2) condloss = limit2; //limit conditional payout
						loss = loss + condloss; // main coverage + extra expense payout
						loss = loss * (1 + scale1); // gross up for debris removal
					}
					else loss = 0;
				}
				else loss = 0;
			}
			else { // if the upper threshold is not 100% then do not include loss = tend in the conditional calculation
				if (loss < tend) {
					if (loss >= tstart) {
						loss = (payout * limit1) - deductible1; //calculate primary payout
						if (loss < 0) loss = 0;
						condloss = loss * scale2; //calculate conditional payout based on primary coverage payout (extra expenses)
						if (condloss > limit2) condloss = limit2; //limit conditional payout
						loss = loss + condloss; // main coverage + extra expense payout
						loss = loss * (1 + scale1); // gross up for debris removal
					}
					else loss = 0;
				}
				else loss = 0;
			}

			if (profile.step_id == 1) {
				x.step_loss = loss;
			}
			else {
				x.step_loss = x.step_loss + loss;
			}
			if (isLast == true) {
				x.loss = x.step_loss;
			}
		}
		break;
		case 31:
		{
			OASIS_FLOAT deductible1 = 0;
			OASIS_FLOAT tstart = 0;
			OASIS_FLOAT tend = 0;
			OASIS_FLOAT payout = 0;
			OASIS_FLOAT scale1 = 0;
			OASIS_FLOAT scale2 = 0;
			OASIS_FLOAT limit2 = 0;
			for (auto y : profile.tc_vec) {
				if (y.tc_id == deductible_1) deductible1 = y.tc_val;
				if (y.tc_id == trigger_start) tstart = y.tc_val;
				if (y.tc_id == trigger_end) tend = y.tc_val;
				if (y.tc_id == payout_start) payout = y.tc_val;
				if (y.tc_id == scale_1) scale1 = y.tc_val;
				if (y.tc_id == scale_2) scale2 = y.tc_val;
				if (y.tc_id == limit_2) limit2 = y.tc_val;

			}
			// Step policy: single step with monetary amount payout, optional deductible, extra expense payout with limit amount, and a gross up factor for debris removal
			OASIS_FLOAT loss = 0;
			OASIS_FLOAT condloss = 0;
			loss = x.loss / x.accumulated_tiv;
			if (tend == 1) { // if the upper threshold is 100% then include loss = tend in the conditional calculation
				if (loss <= tend) {
					if (loss >= tstart) {
						loss = payout - deductible1; //calculate primary payout
						if (loss < 0) loss = 0;
						condloss = loss * scale2; //calculate conditional payout based on primary coverage payout (extra expenses)
						if (condloss > limit2) condloss = limit2; //limit conditional payout
						loss = loss + condloss; // main coverage + extra expense payout
						loss = loss * (1 + scale1); // gross up for debris removal
					}
					else loss = 0;
				}
				else loss = 0;
			}
			else { // if the upper threshold is not 100% then do not include loss = tend in the conditional calculation
				if (loss < tend) {
					if (loss >= tstart) {
						loss = payout - deductible1; //calculate primary payout
						if (loss < 0) loss = 0;
						condloss = loss * scale2; //calculate conditional payout based on primary coverage payout (extra expenses)
						if (condloss > limit2) condloss = limit2; //limit conditional payout
						loss = loss + condloss; // main coverage + extra expense payout
						loss = loss * (1 + scale1); // gross up for debris removal
					}
					else loss = 0;
				}
				else loss = 0;
			}

			if (profile.step_id == 1) {
				x.step_loss = loss;
			}
			else {
				x.step_loss = x.step_loss + loss;
			}
			if (isLast == true) {
				x.loss = x.step_loss;
			}
		}
		break;
		case 32:
		{
			OASIS_FLOAT tstart = 0;
			OASIS_FLOAT payout = 0;
			OASIS_FLOAT scale1 = 0;
			OASIS_FLOAT limit1 = 0;
			OASIS_FLOAT scale2 = 0;
			OASIS_FLOAT limit2 = 0;

			for (auto y : profile.tc_vec) {
				if (y.tc_id == trigger_start) tstart = y.tc_val;
				if (y.tc_id == payout_start) payout = y.tc_val;
				if (y.tc_id == scale_1) scale1 = y.tc_val;
				if (y.tc_id == limit_1) limit1 = y.tc_val;
				if (y.tc_id == scale_2) scale2 = y.tc_val;
				if (y.tc_id == limit_2) limit2 = y.tc_val;
			}
			// Step policy: franchise trigger as a monetary amount with % loss payout, a limit amount and optional extra expenses and scale up factor for debris removal
			OASIS_FLOAT loss = 0;
			OASIS_FLOAT condloss = 0;
			if (x.loss >= tstart) {
				loss = payout * x.loss;
				if (loss > limit1) loss = limit1;
				condloss = loss * scale2; //calculate conditional payout based on primary coverage payout (extra expenses)
				if (condloss > limit2) condloss = limit2; //limit conditional payout
				loss = loss + condloss; // main coverage + extra expense payout
				loss = loss * (1 + scale1);
			}
			else loss = 0;
			
			if (profile.step_id == 1) {
				x.step_loss = loss;
			}
			else {
				x.step_loss = x.step_loss + loss;
			}
			if (isLast == true) {
				x.loss = x.step_loss;
			}


		}
		break;
		case 34:
		{
			OASIS_FLOAT ded = 0;
			OASIS_FLOAT share = 0;
			OASIS_FLOAT att = 0;
			for (auto y : profile.tc_vec) {
				if (y.tc_id == deductible_1) ded = y.tc_val;
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
			loss = loss - att;
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
		case 100:	// noop
		{
			x.loss = x.loss;
		}
		break;
		default:
		{
			fprintf(stderr, "FATAL: Unknown calc rule %d\n", profile.calcrule_id);
		}
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
			//else loss = loss;
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
			x.under_limit = x.under_limit + x.loss - loss;
			x.loss = loss;
		}
		break;
		case 7:
			//  minimum and maximum applied to prior level effective deductible plus deductible with limit
		{
			OASIS_FLOAT ded1 = 0;
			OASIS_FLOAT ded2 = 0;
			OASIS_FLOAT ded3 = 0;
			OASIS_FLOAT lim = 0;
			for (auto y : profile.tc_vec) {
				if (y.tc_id == deductible_1) ded1 = y.tc_val;
				if (y.tc_id == deductible_2) ded2 = y.tc_val;
				if (y.tc_id == deductible_3) ded3 = y.tc_val;
				if (y.tc_id == limit_1) lim = y.tc_val;
			}
			OASIS_FLOAT loss = 0;
			OASIS_FLOAT effective_ded = 0;
			OASIS_FLOAT loss_delta = 0;

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
			OASIS_FLOAT ded1 = 0;
			OASIS_FLOAT ded2 = 0;
			OASIS_FLOAT lim = 0;
			for (auto y : profile.tc_vec) {
				if (y.tc_id == deductible_1) ded1 = y.tc_val;
				if (y.tc_id == deductible_2) ded2 = y.tc_val;
				if (y.tc_id == limit_1) lim = y.tc_val;
			}
			OASIS_FLOAT loss = 0;
			OASIS_FLOAT loss_delta = 0;
			if (x.effective_deductible + ded1 < ded2) { // If effective deductible is less than the minimum, deductible will be increased to the minimum
				loss_delta = x.effective_deductible - ded2; // negative loss change
				if (x.over_limit + x.under_limit > 0) { // if there are prior level limits to reapply
					if (x.under_limit == 0) { // If carried loss is at a prior level limit
						if (-loss_delta > x.over_limit) { // if the loss decrease will take the loss back through the prior level limits
							loss = x.loss + x.over_limit + x.effective_deductible - ded2; //let the loss decrease by the difference between the overlimit and the loss delta
							x.effective_deductible = ded2;
						}
						else loss = x.loss; // no change to the loss because the adjusted loss is still overlimit
						x.effective_deductible = ded2;//update the deductible to carry forward
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
				loss = x.loss - ded1; 
				if (loss < 0) loss = 0; //loss can't go negative
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
			OASIS_FLOAT ded1 = 0;
			OASIS_FLOAT ded3 = 0;
			for (auto y : profile.tc_vec) {
				if (y.tc_id == deductible_1) ded1 = y.tc_val;
				if (y.tc_id == deductible_3) ded3 = y.tc_val;
			}
			// Function 10: Applies a cap on retained loss (maximum deductible)
			OASIS_FLOAT loss = 0;
			OASIS_FLOAT loss_delta = 0;
			if ((x.effective_deductible + ded1) > ded3) { //If effective deductible is more than the maximum, deductible will be reduced to the maximum
				loss_delta = x.effective_deductible - ded3; // loss to increase by the loss_delta
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
				loss = x.loss - ded1; 
				if (loss < 0) loss = 0;
			}
			x.loss = loss;
		}
		break;
		case 11: 
		{
			OASIS_FLOAT ded1 = 0;
			OASIS_FLOAT ded2 = 0;
			for (auto y : profile.tc_vec) {
				if (y.tc_id == deductible_1) ded1 = y.tc_val;
				if (y.tc_id == deductible_2) ded2 = y.tc_val;
			}
			OASIS_FLOAT loss = 0;
			OASIS_FLOAT loss_delta = 0;
			if ((x.effective_deductible + ded1) < ded2) { // If effective deductible is less than the minimum, deductible will be increased to the minimum
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
				loss = x.loss - ded1; //no change to loss if carried deductible is more than the min ded.
				if (loss < 0) loss = 0;
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
						x.under_limit = x.under_limit + x.loss - loss;
						x.loss = loss;
						break;
					}
				}
			}
		}
		break;
		case 13: // insurance only
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

			if (x.effective_deductible + ded1 > ded3) { //If effective deductible is more than the maximum, deductible will be reduced to the maximum
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
				if (x.effective_deductible + ded1 < ded2) { // If effective deductible is less than the minimum, deductible will be increased to the minimum
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
					loss = x.loss - ded1;
					if (loss < 0) loss = 0;
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
			OASIS_FLOAT ded = 0;
			OASIS_FLOAT lim = 0;
			for (auto y : profile.tc_vec) {
				if (y.tc_id == deductible_1) ded = y.tc_val;
				if (y.tc_id == limit_1) lim = y.tc_val;
			}
			
			OASIS_FLOAT loss = x.loss;
			loss = loss - ded;
			if (loss < 0) loss = 0;
			if (loss > (x.loss * lim)) loss = x.loss * lim;
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
			if (ded3 == 0) ded3 = 9999999999;
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
			//else loss = loss;
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
			OASIS_FLOAT lim = 0;
			for (auto y : profile.tc_vec) {
				if (y.tc_id == deductible_1) ded1 = y.tc_val;
				if (y.tc_id == deductible_2) ded2 = y.tc_val;
				if (y.tc_id == deductible_3) ded3 = y.tc_val;
				if (y.tc_id == limit_1) lim = y.tc_val;
			}
			OASIS_FLOAT loss = 0;
			OASIS_FLOAT loss_delta = 0;
			// Applies a min and max ded on effective deductible plus a deductible as an amount.
			if (((x.loss * ded1) + x.effective_deductible) > ded3) { //If carried + ded > max ded
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
				if (((x.loss * ded1) + x.effective_deductible) < ded2) { //If carried + ded < min ded
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
					loss = x.loss - (x.loss * ded1);
					if (loss < 0) loss = 0;
					x.effective_deductible = x.effective_deductible + (x.loss - loss);
					//x.retained_loss = x.retained_loss + (x.loss - loss);		
				}
			if (loss > lim) {
				x.over_limit = loss - lim;
				loss = lim;
			}
			x.under_limit = lim - loss;
			x.loss = loss;
			}
			x.loss = loss;
		}
		break;
		case 33: // insurance only
		{
			OASIS_FLOAT ded = 0;
			OASIS_FLOAT lim = 0;
			for (auto y : profile.tc_vec) {
				if (y.tc_id == deductible_1) ded = y.tc_val;
				if (y.tc_id == limit_1) lim = y.tc_val;
			}
			//Function33: deductible % loss applies before limit 
			OASIS_FLOAT loss = 0;
			loss = x.loss - (ded * x.loss);
			if (loss < 0) loss = 0;
			x.effective_deductible = x.effective_deductible + (x.loss - loss);
			if (loss > lim) loss = lim;
			x.loss = loss;
		}
		break;
		case 34:
		{
			OASIS_FLOAT ded = 0;
			OASIS_FLOAT share = 0;
			OASIS_FLOAT att = 0;
			for (auto y : profile.tc_vec) {
				if (y.tc_id == deductible_1) ded = y.tc_val;
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
			loss = loss - att;
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
		case 35:// insurance only
		{
			OASIS_FLOAT ded1 = 0;
			OASIS_FLOAT ded2 = 0;
			OASIS_FLOAT lim = 0;
			for (auto y : profile.tc_vec) {
				if (y.tc_id == deductible_1) ded1 = y.tc_val;
				if (y.tc_id == deductible_2) ded2 = y.tc_val;
				if (y.tc_id == limit_1) lim = y.tc_val;
			}
			OASIS_FLOAT loss = 0;
			OASIS_FLOAT loss_delta = 0;
			if ((x.effective_deductible + (ded1 * x.loss)) < ded2) { // If effective deductible is less than the minimum, deductible will be increased to the minimum
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
				loss = x.loss - (ded1 * x.loss); //no change to loss if carried deductible is more than the min ded.
				if (loss < 0) loss = 0;
			}
			if (loss > lim) {
				x.over_limit = loss - lim;
				loss = lim;
			}
			x.under_limit = lim - loss;
			x.loss = loss;
		}
		break;
		case 100:	// noop
		{
			x.loss = x.loss;
		}
		break;
		default:
		{
			fprintf(stderr, "FATAL: Unknown calc rule %d\n", profile.calcrule_id);
		}
	}
}

void fmcalc::dofmcalc_normal(std::vector <LossRec> &agg_vec, int layer)
{
	for (LossRec &x : agg_vec) {
		if (x.agg_id == 0) break;
		if (x.agg_id > 0) {
			if (x.loss > 0 || x.retained_loss > 0) {
				if (x.policytc_id > 0) {
					const profile_rec_new &profile = profile_vec_new_[x.policytc_id];
					if (profile.calcrule_id != 100) applycalcrule(profile, x, layer);
				}
			}
		}
	}
}


void fmcalc::dofmcalc_stepped(std::vector <LossRec>& agg_vec, int layer)
{
	for (LossRec& x : agg_vec) {
		if (x.agg_id == 0) break;
		if (x.agg_id > 0) {
			if (x.loss > 0 || x.retained_loss > 0) {
				if (x.policytc_id > 0) {
					auto iter = profile_vec_stepped_[x.policytc_id].cbegin();
					while (iter != profile_vec_stepped_[x.policytc_id].cend()) {
						const profile_rec_new& profile = *(iter);						
						iter++;
						if (iter == profile_vec_stepped_[x.policytc_id].cend()) {
							if (profile.calcrule_id != 100) applycalcrule_stepped(profile, x, layer,true);
						}
						else {
							if (profile.calcrule_id != 100) applycalcrule_stepped(profile, x, layer, false);
						}
					}
				}
			}
		}
	}
}


void fmcalc::dofmcalc(std::vector <LossRec>& agg_vec, int layer)
{
	if (stepped_ == false) dofmcalc_normal(agg_vec, layer);
	else dofmcalc_stepped(agg_vec, layer);
}
void fmcalc::init_profile__stepped_rec(fm_profile_step& f)
{
	profile_rec_new p;
	p.calcrule_id = f.calcrule_id;
	p.step_id = f.step_id;
	switch (p.calcrule_id) {
		case 2:
			add_tc(deductible_1, f.deductible1, p.tc_vec);
			add_tc(limit_1, f.limit1, p.tc_vec);
			add_tc(share_1, f.share1, p.tc_vec);
			add_tc(attachment_1, f.attachment, p.tc_vec);
			break;
		case 12:
			add_tc(deductible_1, f.deductible1, p.tc_vec);
			break;
		case 14:
			add_tc(limit_1, f.limit1, p.tc_vec);
			break;
		case 27:
			add_tc(deductible_1, f.deductible1, p.tc_vec);
			add_tc(limit_2, f.limit2, p.tc_vec);
			add_tc(limit_1, f.limit1, p.tc_vec);
			add_tc(payout_start, f.payout_start, p.tc_vec);
			add_tc(scale_1, f.scale1, p.tc_vec);
			add_tc(scale_2, f.scale2, p.tc_vec);
			add_tc(trigger_start, f.trigger_start, p.tc_vec);
			add_tc(trigger_end, f.trigger_end, p.tc_vec);
			break;
		case 28:
			add_tc(deductible_1, f.deductible1, p.tc_vec);
			add_tc(limit_2, f.limit2, p.tc_vec);
			add_tc(payout_start, f.payout_start, p.tc_vec);
			add_tc(scale_1, f.scale1, p.tc_vec);
			add_tc(scale_2, f.scale2, p.tc_vec);
			add_tc(trigger_start, f.trigger_start, p.tc_vec);
			add_tc(trigger_end, f.trigger_end, p.tc_vec);
			break;
		case 29:
			add_tc(deductible_1, f.deductible1, p.tc_vec);
			add_tc(limit_2, f.limit2, p.tc_vec);
			add_tc(payout_start, f.payout_start, p.tc_vec);
			add_tc(scale_1, f.scale1, p.tc_vec);
			add_tc(scale_2, f.scale2, p.tc_vec);
			add_tc(trigger_start, f.trigger_start, p.tc_vec);
			add_tc(trigger_end, f.trigger_end, p.tc_vec);
			break;
		case 30:
			add_tc(deductible_1, f.deductible1, p.tc_vec);
			add_tc(limit_2, f.limit2, p.tc_vec);
			add_tc(limit_1, f.limit1, p.tc_vec);
			add_tc(payout_start, f.payout_start, p.tc_vec);
			add_tc(scale_1, f.scale1, p.tc_vec);
			add_tc(scale_2, f.scale2, p.tc_vec);
			add_tc(trigger_start, f.trigger_start, p.tc_vec);
			add_tc(trigger_end, f.trigger_end, p.tc_vec);
			break;
		case 31:
			add_tc(deductible_1, f.deductible1, p.tc_vec);
			add_tc(limit_2, f.limit2, p.tc_vec);
			add_tc(payout_start, f.payout_start, p.tc_vec);
			add_tc(scale_1, f.scale1, p.tc_vec);
			add_tc(scale_2, f.scale2, p.tc_vec);
			add_tc(trigger_start, f.trigger_start, p.tc_vec);
			add_tc(trigger_end, f.trigger_end, p.tc_vec);
			break;
		case 32:
			add_tc(payout_start, f.payout_start, p.tc_vec);
			add_tc(scale_1, f.scale1, p.tc_vec);
			add_tc(trigger_start, f.trigger_start, p.tc_vec);
			add_tc(limit_1, f.limit1, p.tc_vec);
			add_tc(limit_2, f.limit2, p.tc_vec);
			add_tc(scale_2, f.scale2, p.tc_vec);
			break;
		case 34:
			add_tc(deductible_1, f.deductible1, p.tc_vec);
			add_tc(share_1, f.share1, p.tc_vec);
			add_tc(attachment_1, f.attachment, p.tc_vec);
			break;
		case 100:
			break;
		default:
		{
			fprintf(stderr, "FATAL: Invalid calc rule for stepped policies\n");
			exit(EXIT_FAILURE);
		}
	}
	if (profile_vec_stepped_.size() < ((size_t) f.profile_id) + 1 ) {
		profile_vec_stepped_.resize((size_t) f.profile_id + 1);
	}
	profile_vec_stepped_[f.profile_id].push_back(p);
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
		case 7:
			add_tc(deductible_1, f.deductible1, p.tc_vec);
			add_tc(deductible_2, f.deductible2, p.tc_vec);
			add_tc(deductible_3, f.deductible3, p.tc_vec);
			add_tc(limit_1, f.limit, p.tc_vec);
			break;
		case 8:
			add_tc(deductible_1, f.deductible1, p.tc_vec);
			add_tc(deductible_2, f.deductible2, p.tc_vec);
			add_tc(limit_1, f.limit, p.tc_vec);
			break;
		case 9:
			add_tc(deductible_1, f.deductible1, p.tc_vec);
			add_tc(limit_1, f.limit, p.tc_vec);
			break;
		case 10:
			add_tc(deductible_1, f.deductible1, p.tc_vec);
			add_tc(deductible_3, f.deductible3, p.tc_vec);
			break;
		case 11:
			add_tc(deductible_1, f.deductible1, p.tc_vec);
			add_tc(deductible_2, f.deductible2, p.tc_vec);
			break;
		case 12:
			add_tc(deductible_1, f.deductible1, p.tc_vec);
			break;
		case 13:
			add_tc(deductible_1, f.deductible1, p.tc_vec);
			add_tc(deductible_2, f.deductible2, p.tc_vec);
			add_tc(deductible_3, f.deductible3, p.tc_vec);
			break;
		case 14:
			add_tc(limit_1, f.limit, p.tc_vec);
			break;
		case 15:
			add_tc(deductible_1, f.deductible1, p.tc_vec);
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
		case 26:
			add_tc(deductible_1, f.deductible1, p.tc_vec);
			add_tc(deductible_2, f.deductible2, p.tc_vec);
			add_tc(deductible_3, f.deductible3, p.tc_vec);
			add_tc(limit_1, f.limit, p.tc_vec);
			break;
		case 33:
			add_tc(deductible_1, f.deductible1, p.tc_vec);
			add_tc(limit_1, f.limit, p.tc_vec);
			break;
		case 34:
			add_tc(deductible_1, f.deductible1, p.tc_vec);
			add_tc(share_1, f.share1, p.tc_vec);
			add_tc(attachment_1, f.attachment, p.tc_vec);
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
	if (profile_vec_new_.size() < (size_t)f.profile_id + 1) {
		profile_vec_new_.resize(f.profile_id + 1);
	}
	profile_vec_new_[f.profile_id] = p;
}

void fmcalc::init_profile_step()
{
	FILE* fin = NULL;
	std::string file = FMPROFILE_FILE_STEP;
	if (inputpath_.length() > 0) {
		file = inputpath_ + file.substr(5);
	}
	fin = fopen(file.c_str(), "rb");
	if (fin == NULL) {
		fprintf(stderr, "%s: cannot open %s\n", __func__, file.c_str());
		exit(EXIT_FAILURE);
	}
	fm_profile_step f;
	size_t i = fread(&f, sizeof(f), 1, fin);
	while (i != 0) {
		init_profile__stepped_rec(f);
		if (noop_profile_id < f.profile_id) noop_profile_id = f.profile_id;
		i = fread(&f, sizeof(f), 1, fin);
	}
	noop_profile_id++;
	fm_profile_step d;	// dummy
	d.profile_id = noop_profile_id;
	d.calcrule_id = 100; // noop
	init_profile__stepped_rec(d);
	fclose(fin);
}
void fmcalc::init_profile()
{
	FILE *fin = NULL;
	std::string file = FMPROFILE_FILE;
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
	//d.calcrule_id = 14;
	d.calcrule_id = 100; // noop
	init_profile_rec(d);
	fclose(fin);
}
