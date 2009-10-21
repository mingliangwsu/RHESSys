/*--------------------------------------------------------------*/
/* 								*/
/*		patch_daily_I					*/
/*								*/
/*	NAME							*/
/*	patch_daily_I 						*/
/*		 - performs cycling of patch state vars		*/
/*			at the START OF THE DAY			*/ 
/*								*/
/*	SYNOPSIS 						*/
/*	void patch_daily_I(					*/
/*			 struct patch_object	,		*/
/*			 struct command_line_object ,		*/
/*			 struct tec_entry,			*/
/*			 struct date)				*/
/*								*/
/*	OPTIONS							*/
/*	struct	world_object *world,				*/
/*	struct	basin_object *basin,				*/
/*	struct 	hillslope_object *hillslope,			*/
/*	struct  zone_object *zone,				*/
/*	struct patch_object *patch,				*/
/*	struct command_line_object *command_line,		*/
/*	struct command_line_object *command_line,		*/
/*	struct	tec_entry	*event,				*/
/*	struct	date current_date - local time (?)		*/
/*								*/
/*	DESCRIPTION						*/
/*								*/
/*	This routine performs simulation cycles on an identified*/
/*	canopy_stata in the patch.				*/
/*								*/
/*	PROGRAMMER NOTES					*/
/*								*/
/*	March 12, 1997	C.Tague					*/
/*	- added calculation for patch effective lai		*/
/*								*/
/*								*/
/*	Sept 15 1997	RAF					*/
/*	Substantially modified accounting of current water	*/
/*	equivalent depth to sat zone and unsat_storage		*/
/*								*/
/*	Sept 29 1997 CT						*/
/*	switched above to an implementation using sat_deficit	*/
/*	as the TOP_model control volume - see discussion 	*/
/*	there							*/
/*								*/
/*	Oct 22 1997 CT						*/
/*								*/
/*	unsat storage now subtracted from sat_deficit after	*/
/*	return flow calculated - in previous version this	*/
/*	step was missing which  results in a 			*/
/*	serious over-estimation of sat_deficit after		*/
/*	return flow events					*/	
/*								*/
/*	Feb 2 1998 RAF						*/
/*	Included potential exfiltration module.			*/
/*								*/
/*	April 28 1998 RAF					*/
/*	Excluded stratum of height 0 from computation of	*/
/*	effective LAI of site.					*/
/*--------------------------------------------------------------*/
#include "rhessys.h"

void		patch_daily_I(
						  struct	world_object *world,
						  struct	basin_object *basin,
						  struct 	hillslope_object *hillslope,
						  struct  zone_object *zone,
						  struct patch_object *patch,
						  struct command_line_object *command_line,
						  struct	tec_entry	*event,
						  struct	date current_date)
{
	/*--------------------------------------------------------------*/
	/*  Local Function Declarations.                                */
	/*--------------------------------------------------------------*/
	void   canopy_stratum_daily_I(
		struct	world_object *,
		struct	basin_object *,
		struct 	hillslope_object *,
		struct  zone_object *,
		struct	patch_object *,
		struct canopy_strata_object *,
		struct command_line_object *,
		struct tec_entry *,
		struct date);
	
	double	compute_layer_field_capacity(
		int,
		int,
		double,
		double,
		double,
		double,
		double,
		double,
		double,
		double);
	
	
	double	compute_delta_water(
		int,
		double,
		double,
		double,
		double,
		double);

	double	compute_z_final(
		int,
		double,
		double,
		double,
		double,
		double);

	int	update_rootzone_moist(
		struct patch_object	*,
		struct	rooting_zone_object	*,
		struct command_line_object *);
	
	double	compute_capillary_rise(
		int,
		double,
		double,
		double,
		double,
		double);


	double  compute_potential_exfiltration(
		int,
		double,
		double,
		double,
		double,
		double,
		double,
		double,
		double);
	
	double  compute_soil_water_potential(
		int,
		int,
		double,
		double,
		double,
		double,
		double,
		double,
		double,
		double);
	
	int  compute_potential_decomp(
		double,
		double,
		double,
		double,
		double,
		struct  soil_c_object   *,
		struct  soil_n_object   *,
		struct  litter_c_object *,
		struct  litter_n_object *,
		struct  cdayflux_patch_struct *,
		struct  ndayflux_patch_struct *);
	
	void    sort_patch_layers(struct patch_object *);
	
	int	zero_patch_daily_flux(
		struct	patch_object *,
		struct  cdayflux_patch_struct *,
		struct  ndayflux_patch_struct *);
	
	void	update_litter_interception_capacity(
		double,
		struct  litter_c_object *,
		struct  litter_object *);
	
	/*--------------------------------------------------------------*/
	/*  Local variable definition.                                  */
	/*--------------------------------------------------------------*/
	int	layer;
	int	stratum;
	double	count,rzm;
	struct  canopy_strata_object *strata;
	/*--------------------------------------------------------------*/
	/*	zero out daily fluxes					*/
	/*--------------------------------------------------------------*/
	if (zero_patch_daily_flux(patch, &(patch[0].cdf), &(patch[0].ndf))){
		fprintf(stderr,
			"Error in zero_patch_daily_flux() from patch_daily_I.c... Exiting\n");
		exit(1);
	}

	/*-----------------------------------------------------*/
	/*  Compute potential saturation for rootzone layer   */
	/*-----------------------------------------------------*/			
	if (patch[0].rootzone.depth > ZERO) 
	patch[0].rootzone.potential_sat = compute_delta_water(
		command_line[0].verbose_flag,
		patch[0].soil_defaults[0][0].porosity_0,
		patch[0].soil_defaults[0][0].porosity_decay,
		patch[0].soil_defaults[0][0].soil_depth,
		patch[0].rootzone.depth, 0.0);			
	else patch[0].rootzone.potential_sat = 0.0;


	/*--------------------------------------------------------------*/
	/*	compute new field capacity				*/
	/*--------------------------------------------------------------*/

	if (patch[0].sat_deficit_z < patch[0].rootzone.depth)  {
		patch[0].rootzone.field_capacity = compute_layer_field_capacity(
			command_line[0].verbose_flag,
			patch[0].soil_defaults[0][0].theta_psi_curve,
			patch[0].soil_defaults[0][0].psi_air_entry,
			patch[0].soil_defaults[0][0].pore_size_index,
			patch[0].soil_defaults[0][0].p3,
			patch[0].soil_defaults[0][0].porosity_0,
			patch[0].soil_defaults[0][0].porosity_decay,
			patch[0].sat_deficit,
			patch[0].rootzone.potential_sat, 0.0);				
			
		patch[0].field_capacity = 0.0;
	}
	else  {
		patch[0].rootzone.field_capacity = compute_layer_field_capacity(
			command_line[0].verbose_flag,
			patch[0].soil_defaults[0][0].theta_psi_curve,
			patch[0].soil_defaults[0][0].psi_air_entry,
			patch[0].soil_defaults[0][0].pore_size_index,
			patch[0].soil_defaults[0][0].p3,
			patch[0].soil_defaults[0][0].porosity_0,
			patch[0].soil_defaults[0][0].porosity_decay,
			patch[0].sat_deficit,
			patch[0].rootzone.potential_sat, 0.0);	

		patch[0].field_capacity = compute_layer_field_capacity(
			command_line[0].verbose_flag,
			patch[0].soil_defaults[0][0].theta_psi_curve,
			patch[0].soil_defaults[0][0].psi_air_entry,
			patch[0].soil_defaults[0][0].pore_size_index,
			patch[0].soil_defaults[0][0].p3,
			patch[0].soil_defaults[0][0].porosity_0,
			patch[0].soil_defaults[0][0].porosity_decay,
			patch[0].sat_deficit,
			patch[0].sat_deficit, 0.0) - patch[0].rootzone.field_capacity;
	}



	/*--------------------------------------------------------------*/
	/*	Estimate potential cap rise.				*/
	/*	limited to water in sat zone.				*/
	/*--------------------------------------------------------------*/
	patch[0].potential_cap_rise =	compute_capillary_rise(
		command_line[0].verbose_flag,
		patch[0].sat_deficit_z,
		patch[0].soil_defaults[0][0].psi_air_entry,
		patch[0].soil_defaults[0][0].pore_size_index,
		patch[0].soil_defaults[0][0].mz_v,
		patch[0].soil_defaults[0][0].Ksat_0_v );
	if (patch[0].potential_cap_rise < ZERO)
		patch[0].potential_cap_rise = 0.0;
	patch[0].cap_rise=0.0;



	/*--------------------------------------------------------------*/
	/*	Compute the max exfiltration rate.			*/
	/*								*/
	/*	First check if we are saturated.  If so the 		*/
	/*	potential exfiltration rate is the capillary rise rate.	*/
	/*								*/
	/*	If we are within the active unsat zone then we assume	*/
	/*	that the potential exfiltration rate is the minimum of	*/
	/*	the computed exfiltration rate and the potential cap	*/
	/*	rise - i.e. hydrologic connectivity between surface and	*/
	/*	water table.						*/
	/*								*/
	/*--------------------------------------------------------------*/
	if ( patch[0].sat_deficit_z <= patch[0].soil_defaults[0][0].psi_air_entry ){
		patch[0].potential_exfiltration = patch[0].potential_cap_rise;
	}
	else{
		if ( patch[0].soil_defaults[0][0].active_zone_z < patch[0].sat_deficit_z ){
			/*--------------------------------------------------------------*/
			/*	Estimate potential exfiltration from active zone 	*/
			/*--------------------------------------------------------------*/
			patch[0].potential_exfiltration = compute_potential_exfiltration(
				command_line[0].verbose_flag,
				patch[0].rootzone.S,
				patch[0].soil_defaults[0][0].active_zone_z,
				patch[0].soil_defaults[0][0].Ksat_0_v,
				patch[0].soil_defaults[0][0].mz_v,
				patch[0].soil_defaults[0][0].psi_air_entry,
				patch[0].soil_defaults[0][0].pore_size_index,
				patch[0].soil_defaults[0][0].porosity_decay,
				patch[0].soil_defaults[0][0].porosity_0);
		}
		else {
			/*--------------------------------------------------------------*/
			/*	Estimate potential exfiltration from active zone 	*/
			/*--------------------------------------------------------------*/
			patch[0].potential_exfiltration = compute_potential_exfiltration(
				command_line[0].verbose_flag,
				patch[0].rootzone.S,
				patch[0].sat_deficit_z,
				patch[0].soil_defaults[0][0].Ksat_0_v,
				patch[0].soil_defaults[0][0].mz_v,
				patch[0].soil_defaults[0][0].psi_air_entry,
				patch[0].soil_defaults[0][0].pore_size_index,
				patch[0].soil_defaults[0][0].porosity_decay,
				patch[0].soil_defaults[0][0].porosity_0);
		}
	}


	
	/*--------------------------------------------------------------*/
	/*	Cycle through the canopy layers.			*/
	/*--------------------------------------------------------------*/
	for ( layer=0 ; layer<patch[0].num_layers; layer++ ){
		/*--------------------------------------------------------------*/
		/*	Cycle through the canopy strata				*/
		/*--------------------------------------------------------------*/
		for ( stratum=0 ; stratum<patch[0].layers[layer].count; stratum++ ){

			strata = patch[0].canopy_strata[(patch[0].layers[layer].strata[stratum])];
			patch[0].preday_rain_stored += strata->cover_fraction * strata->rain_stored;
			patch[0].preday_snow_stored += strata->cover_fraction * strata->snow_stored;
			canopy_stratum_daily_I(
				world,
				basin,
				hillslope,
				zone,
				patch,
				patch[0].canopy_strata[(patch[0].layers[layer].strata[stratum])],
				command_line,
				event,
				current_date );
		}
	}
	/*--------------------------------------------------------------*/
	/*	Calculate effective patch lai from stratum					*/
	/*	- for later use by zone_daily_F								*/
	/*      Accumulate root biomass for patch soil -
	/*      required for N updake from soil                         */
	/*	also determine total plant carbon			*/
	/*	- if grow option is specified				*/
	/*--------------------------------------------------------------*/
	patch[0].effective_lai = 0.0;
	patch[0].soil_cs.frootc = 0.0;
	patch[0].rootzone.depth = 0.0;
	count = 0.0;
	for ( stratum=0 ; stratum<patch[0].num_canopy_strata; stratum++){
		patch[0].effective_lai += patch[0].canopy_strata[stratum][0].epv.proj_lai;
		if (command_line[0].grow_flag > 0) {
			patch[0].soil_cs.frootc
				+= patch[0].canopy_strata[stratum][0].cover_fraction
				* patch[0].canopy_strata[stratum][0].cs.frootc;
			patch[0].preday_totalc
				+= patch[0].canopy_strata[stratum][0].cover_fraction
				* patch[0].canopy_strata[stratum][0].cs.preday_totalc;
			patch[0].preday_totaln
				+= patch[0].canopy_strata[stratum][0].cover_fraction
				* patch[0].canopy_strata[stratum][0].ns.preday_totaln;
				
				
		}
		patch[0].rootzone.depth = max(patch[0].rootzone.depth, 
			 patch[0].canopy_strata[stratum][0].rootzone.depth);
	}
	patch[0].effective_lai = patch[0].effective_lai / patch[0].num_canopy_strata;
	/*--------------------------------------------------------------*/
	/*	re-sort patch layers to account for any changes in 	*/
	/*	height							*/
	/*------------------------------------------------------------------------*/
	sort_patch_layers(patch);


	/*------------------------------------------------------------------------*/
	/*	compute current soil moisture potential					*/
	/*	do this before nitrogen updake occurs later in the day			*/
	/*------------------------------------------------------------------------*/

	if (patch[0].sat_deficit > patch[0].rootzone.potential_sat)
		rzm = patch[0].rz_storage;
	else
		rzm = patch[0].rz_storage+patch[0].rootzone.potential_sat-patch[0].sat_deficit;

	patch[0].psi = compute_soil_water_potential(
		command_line[0].verbose_flag,
		patch[0].soil_defaults[0][0].theta_psi_curve,
		patch[0].soil_defaults[0][0].psi_air_entry,
		patch[0].soil_defaults[0][0].pore_size_index,
		patch[0].soil_defaults[0][0].porosity_0,
		patch[0].soil_defaults[0][0].porosity_decay,
		patch[0].soil_defaults[0][0].psi_max,
		patch[0].rootzone.potential_sat,
		rzm,
		patch[0].Tsoil);


	if (command_line[0].grow_flag > 0) {
		update_litter_interception_capacity(
			patch[0].litter.moist_coef,
			&(patch[0].litter_cs),
			&(patch[0].litter));

		if (compute_potential_decomp(
			patch[0].Tsoil,
			patch[0].soil_defaults[0][0].psi_max,
			patch[0].soil_defaults[0][0].psi_air_entry,
			patch[0].rootzone.S,
			patch[0].std,
			&(patch[0].soil_cs),
			&(patch[0].soil_ns),
			&(patch[0].litter_cs),
			&(patch[0].litter_ns),
			&(patch[0].cdf),
			&(patch[0].ndf)
			) != 0){
			fprintf(stderr,"fATAL ERROR: in compute_potential_decomp() ... Exiting\n");
			exit(1);
		}
	}
	return;
}/*end patch_daily_I.c*/
