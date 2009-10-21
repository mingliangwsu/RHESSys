/*--------------------------------------------------------------*/
/* 																*/
/*					output_yearly_patch						*/
/*																*/
/*	output_yearly_patch - creates output files objects.		*/
/*																*/
/*	NAME														*/
/*	output_yearly_patch - outputs current contents of a patch.			*/
/*																*/
/*	SYNOPSIS													*/
/*	void	output_yearly_patch( int basinID, int hillID, int zoneID,								*/
/*					struct	patch_object	*patch,				*/
/*					struct	date	date,  						*/
/*					FILE 	*outfile)							*/
/*																*/
/*	OPTIONS														*/
/*																*/
/*	DESCRIPTION													*/
/*																*/
/*	outputs spatial structure according to commandline			*/
/*	specifications to specific files							*/
/*																*/
/*	PROGRAMMER NOTES											*/
/*																*/
/*	We only permit one fileset per spatial modelling level.     */
/*	Each fileset has one file for each timestep.  				*/
/*																*/
/*--------------------------------------------------------------*/
#include <stdio.h>
#include "rhessys.h"

void	output_yearly_patch(
				int basinID, int hillID, int zoneID,
				struct	patch_object	*patch,
				struct	date	current_date,
				FILE *outfile)
{
	/*------------------------------------------------------*/
	/*	Local Function Declarations.						*/
	/*------------------------------------------------------*/
	
	/*------------------------------------------------------*/
	/*	Local Variable Definition. 							*/
	/*------------------------------------------------------*/

	if (patch[0].acc_year.length > 0)
		patch[0].acc_year.theta /= patch[0].acc_year.length;


	fprintf(outfile,"%4d %d %d %d %d %d %d %d %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %d %d %lf %lf\n",
			current_date.year,
			basinID,
			hillID,
			zoneID,
			patch[0].ID,
			patch[0].acc_year.num_threshold,
			patch[0].acc_year.peaksweday,
			patch[0].acc_year.meltday,
			patch[0].acc_year.leach * 1000.0,
			patch[0].acc_year.denitrif * 1000.0,
			patch[0].acc_year.DOC_loss * 1000.0,
			patch[0].acc_year.DON_loss * 1000.0,
			patch[0].acc_year.psn * 1000.0,
			patch[0].acc_year.trans * 1000.0,
			patch[0].acc_year.et * 1000.0,
			patch[0].acc_year.lai,
			patch[0].acc_year.nitrif * 1000.0,
			patch[0].acc_year.mineralized * 1000.0,
			patch[0].acc_year.uptake * 1000.0,
			patch[0].acc_year.theta,
			patch[0].acc_year.sm_deficit*1000.0,
			patch[0].acc_year.snowpack * 1000.0,
			patch[0].acc_year.maxtrans * 1000.0,
			patch[0].acc_year.maxpet * 1000.0,
			patch[0].acc_year.rec_wyd,
			patch[0].acc_year.rec_pet_wyd,
			patch[0].area, patch[0].acc_year.PET*1000.0);

	/*--------------------------------------------------------------*/
	/*      reset accumulator variables                             */
	/*--------------------------------------------------------------*/
	patch[0].acc_year.num_threshold = 0;
	patch[0].acc_year.peaksweday = 0;
	patch[0].acc_year.meltday = 0;
	patch[0].acc_year.leach = 0.0;
	patch[0].acc_year.denitrif = 0.0;
	patch[0].acc_year.nitrif = 0.0;
	patch[0].acc_year.DOC_loss = 0.0;
	patch[0].acc_year.DON_loss = 0.0;
	patch[0].acc_year.psn = 0.0;
	patch[0].acc_year.et = 0.0;
	patch[0].acc_year.trans = 0.0;
	patch[0].acc_year.PET = 0.0;
	patch[0].acc_year.lai = 0.0;
	patch[0].acc_year.mineralized = 0.0;
	patch[0].acc_year.uptake = 0.0;
	patch[0].acc_year.theta = 0.0;
	patch[0].acc_year.sm_deficit = 0.0;
	patch[0].acc_year.snowpack = 0.0;
	patch[0].acc_year.length = 0;
	patch[0].acc_year.wyd = 0;
	patch[0].acc_year.rec_wyd = 0;
	patch[0].acc_year.day7trans = 0;
	patch[0].acc_year.maxtrans = 0;
	return;

} /*end output_csv_yearly_patch*/

