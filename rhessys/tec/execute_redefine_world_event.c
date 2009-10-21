/*--------------------------------------------------------------*/
/* 																*/
/*					execute_redefine_world_event								*/
/*																*/
/*	execute_redefine_world_event.c - creates a world object					*/
/*																*/
/*	NAME														*/
/*	execute_redefine_world_event.c - creates a world object					*/
/*																*/
/*	SYNOPSIS													*/
/*	void execute_redefine_world_event_line(world, &command_line)	*/
/*																*/
/*	OPTIONS														*/
/*																*/
/*	DESCRIPTION													*/
/*																*/
/*	execute_redefines stratum objects based on an input file			*/
/*	input file is a worldfile in standard format -       	*/
/*	it will be used to reset all state variable unless state variable */
/*	in the input file is -9999;			*/
/*	note, spatial structure cannot change - thus input worldfile must */
/*	have the same basin,hill,zone,patch,strata IDs as current world */

/*	PROGRAMMER NOTES											*/
/*																*/
/*	Original code, January 15, 2003.							*/
/*--------------------------------------------------------------*/
#include <stdio.h>
#include <string.h>
#include "rhessys.h"

void	execute_redefine_world_event(struct world_object *world,
									  struct command_line_object *command_line,
									  struct date current_date)
{
	/*--------------------------------------------------------------*/
	/*	Local function definition.									*/
	/*--------------------------------------------------------------*/
	void input_new_strata( struct command_line_object *,
		FILE *,
		int,
		struct base_station_object **,
		struct default_object *,
		struct patch_object *,
		struct canopy_strata_object *);
	void input_new_patch( struct command_line_object *,
		FILE *,
		int,
		struct base_station_object **,
		struct default_object *,
		struct patch_object *);
	void input_new_zone( struct command_line_object *,
		FILE *,
		int,
		struct base_station_object **,
		struct default_object *,
		struct zone_object *);
	void input_new_hillslope( struct command_line_object *,
		FILE *,
		int,
		struct base_station_object **,
		struct default_object *,
		struct hillslope_object *);
	void input_new_basin( struct command_line_object *,
		FILE *,
		int,
		struct base_station_object **,
		struct default_object *,
		struct basin_object *);
	void compute_mean_hillslope_parameters( struct hillslope_object *);
	struct canopy_strata_object	*find_stratum_in_patch( int, 
		struct patch_object *);
	struct patch_object	*find_patch_in_zone( int, 
		struct zone_object *);
	struct zone_object	*find_zone_in_hillslope( int, 
		struct hillslope_object *);
	struct hillslope_object	*find_hillslope_in_basin( int, 
		struct basin_object *);
	struct basin_object	*find_basin( int, 
		struct world_object *);
	/*--------------------------------------------------------------*/
	/*	Local variable definition.									*/
	/*--------------------------------------------------------------*/
	FILE	*world_input_file;
	int	b,h,z,p,c;
	int	basin_ID, world_ID, hill_ID, zone_ID, patch_ID, stratum_ID;
	int	num_basin, num_hill, num_zone, num_patch, num_stratum;
	char	world_input_filename[MAXSTR];
	char	record[MAXSTR];
	char	ext[11];
	struct	canopy_strata_object	*stratum;
	struct	patch_object	*patch;
	struct	zone_object	*zone;
	struct	hillslope_object	*hillslope;
	struct	basin_object	*basin;
	/*--------------------------------------------------------------*/
	/*	Try to open the world file in read mode.					*/
	/*--------------------------------------------------------------*/
	sprintf(ext,".Y%4dM%dD%dH%d",current_date.year,
		current_date.month,
		current_date.day,
		current_date.hour);
	strcpy(world_input_filename, command_line[0].world_filename);
	strcat(world_input_filename, ext);
	if ( (world_input_file = fopen(world_input_filename,"r")) == NULL ){
		fprintf(stderr,
			"FATAL ERROR:  Cannot open world  execute_redefine input file %s\n",
			world_input_filename);
		exit(0);
	} /*end if*/

	printf("\n Redefine using %s", world_input_filename);
	/*--------------------------------------------------------------*/
	/*	Read in the world ID.							*/
	/*--------------------------------------------------------------*/
	fscanf(world_input_file,"%d",&world_ID);
	read_record(world_input_file, record);
	/*--------------------------------------------------------------*/
	/*	Read in the number of basin	files.							*/
	/*--------------------------------------------------------------*/
	fscanf(world_input_file,"%d",&num_basin);
	read_record(world_input_file, record);
	/*--------------------------------------------------------------*/
	/*	Construct the basins. 										*/
	/*--------------------------------------------------------------*/
	for (b=0; b < num_basin; b++ ){
		fscanf(world_input_file,"%d",&basin_ID);
		read_record(world_input_file, record);
		basin = find_basin( basin_ID,
			world);
		input_new_basin(command_line, world_input_file,
			world[0].num_base_stations,
			world[0].base_stations,
			world[0].defaults,
			basin);
		fscanf(world_input_file,"%d",&num_hill);
		read_record(world_input_file, record);
		for ( h = 0; h < num_hill; h++){
			fscanf(world_input_file,"%d",&hill_ID);
			read_record(world_input_file, record);
			hillslope = find_hillslope_in_basin( hill_ID,
				basin);
			input_new_hillslope(command_line, world_input_file,
				world[0].num_base_stations,
				world[0].base_stations,
				world[0].defaults,
				hillslope);
			fscanf(world_input_file,"%d",&num_zone);
			read_record(world_input_file, record);
			for ( z=0; z < num_zone; z++) {
				fscanf(world_input_file,"%d",&zone_ID);
				read_record(world_input_file, record);
				zone = find_zone_in_hillslope( zone_ID,
					hillslope);
				input_new_zone(command_line, world_input_file,
					world[0].num_base_stations,
					world[0].base_stations,
					world[0].defaults,
					zone);
				fscanf(world_input_file, "%d",&num_patch);
				read_record(world_input_file, record);
				for (p=0; p < num_patch; p++) {
					fscanf(world_input_file,"%d",&patch_ID);
					read_record(world_input_file, record);
					patch = find_patch_in_zone( patch_ID,
						zone);
					input_new_patch(command_line, world_input_file,
						world[0].num_base_stations,
						world[0].base_stations,
						world[0].defaults,
						patch);
					fscanf(world_input_file, "%d",&num_stratum);
					read_record(world_input_file, record);
					for (c=0; c < num_stratum; c++) {
						fscanf(world_input_file,"%d",&stratum_ID);
						read_record(world_input_file, record);
						stratum = find_stratum_in_patch(stratum_ID,
							patch);
						input_new_strata(command_line, world_input_file,
							world[0].num_base_stations,
							world[0].base_stations,
							world[0].defaults,
							patch,
							stratum);
					} /* end c */
			/*--------------------------------------------------------------*/
			/*	re-sort patch layers to account for any changes in 	*/
			/*	height							*/
			/*--------------------------------------------------------------*/
					sort_patch_layers(patch);
				} /* end p */
			} /* end z */
		compute_mean_hillslope_parameters(hillslope);	  
		} /* end h */
	} /*end b */
	/*--------------------------------------------------------------*/
	/*	Close the world_input_file.										*/
	/*--------------------------------------------------------------*/
	if ( fclose(world_input_file) != 0 )
		exit(0);
	return;
} /*end execute_redefine_world_event.c*/
