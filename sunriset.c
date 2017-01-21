/* +++Date last modified: 05-Jul-1997
   Updated comments, 05-Aug-2013

   This version modified by GK Grotsky
   1/20/2017

SUNRISET.C - computes Sun rise/set times, start/end of twilight, and
            the length of the day at any date and latitude

Written as DAYLEN.C, 1989-08-16

Modified to SUNRISET.C, 1992-12-01

(c) Paul Schlyter, 1989, 1992

Released to the public domain by Paul Schlyter, December 1992


*/

#include "sunriset.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <string.h>

int main(void)
{
    time_t rawtime;
    struct tm* pts;
    int year, month, day, hours, hours24, minutes, seconds, inputChoice, UTCOffset;
    char monthS[] = "  ", dayS[] = "  ", hoursS[] = "  ", minutesS[] = "  ", secondsS[] = "  ";
    char selectedLocation[30];
    bool defaultLocation = false;
    bool doDST = true;
    bool bypassGMT = false;
    bool addOffset = false;
    bool sameasET = false;
    whereDST_t whereDST = USA;

    double lon = -77.05d, lat = 39.48333d; // Washington DC
    double daylen, civlen, nautlen, astrlen;
    double rise, set, civ_start, civ_end, naut_start, naut_end,
    astr_start, astr_end;
    int    rs, civ, naut, astr;
    char buf[80];
    char newDay[10];
    char displayChoices[] = "\n\
    1)  Washington DC\n\
    2)  Los Angeles California\n\
    3)  New York City\n\
    4)  Seattle Washington\n\
    5)  Anchorage Alaska\n\
    6)  Honolulu Hawaii\n\
    7)  Greenwich England\n\
    8)  Reykjavik Iceland\n\
    9)  Tokyo Japan\n\
    10) Station Nord Greenland\n\
    11) Moscow Russia\n\
    12) Sydney Australia\n\
    13) Rome Italy\n\
    14) Hamburg Germany\n\
    15) Alert Canada\n\
    16) Enter any location\n";

    while (1)
    {
        printf("%s", displayChoices);
        printf("Enter a number from the above locations: " );
        fgets(buf, sizeof(buf), stdin);
        sscanf(buf, "%d", &inputChoice);
        
        switch (inputChoice)
        {
        case WashingtonDC:
            lat = 39.48333d; 
            lon = -77.05d;
            strcpy(selectedLocation, "Washington DC");
            doDST = true;
            whereDST = USA;
            break;
        case LosAngelesCalifornia:
            lat = 34.0522d; 
            lon = -118.2437d;
            strcpy(selectedLocation, "Los Angeles California");
            doDST = true;
            whereDST = USA;
            break;
        case NewYorkCity:
            lat = 40.7128d; 
            lon = -74.0059d;
            strcpy(selectedLocation, "New York City");
            doDST = true;
            whereDST = USA;
            break;
        case SeattleWashington:
            lat = 47.6062d; 
            lon = -122.3321d;
            strcpy(selectedLocation, "Seattle Washington");
            doDST = true;
            whereDST = USA;
            break;
        case AnchorageAlaska:
            lat = 61.2181d; 
            lon = -149.9003d;
            strcpy(selectedLocation, "Anchorage Alaska");
            doDST = true;
            whereDST = USA;
            break;
        case HonoluluHawaii:
            lat = 21.3069d; 
            lon = -157.8583d;
            strcpy(selectedLocation, "Honolulu Hawaii");
            doDST = false; // Does not observe DST
            whereDST = USA;
            break;
        case GreenwichEngland:
            lat = 51.4826d; 
            lon = -0.0077d;
            strcpy(selectedLocation, "Greenwich England");
            doDST = false; // Does not observe DST
            whereDST = someWhereElse;
            break;
        case ReykjavikIceland:
            lat = 64.135666d; 
            lon = -21.862675d;
            strcpy(selectedLocation, "Reykjavik Iceland");
            doDST = false; // Does not observe DST
            whereDST = someWhereElse;
            bypassGMT = true; // Same time zone as GMT
            break;
        case TokyoJapan:
            lat = 35.6895d; 
            lon = 139.6917d;
            strcpy(selectedLocation, "Tokyo Japan");
            doDST = false; // Does not observe DST
            whereDST = Asia;
            break;
        case StationNordGreenland:
            lat = 81.716667d; 
            lon = 16.66666d;
            strcpy(selectedLocation, "Station Nord Greenland");
            bypassGMT = true; // Same time zone as GMT
            doDST = false; // Does not observe DST
            whereDST = someWhereElse;
            break;
        case MoscowRussia:
            lat = 55.7558d; 
            lon = 37.6173d;
            strcpy(selectedLocation, "Moscow Russia");
            doDST = false; // Does not observe DST
            whereDST = someWhereElse;
            break;
        case SydneyAustralia:
            lat = -33.8688d; 
            lon = 151.2093d;
            strcpy(selectedLocation, "Sydney Australia");
            addOffset = true; // Australia has it own DST which I'm not going to implement now :-(
            doDST = true;
            whereDST = someWhereElse;
            break;
        case RomeItaly:
            lat = 41.9028d; 
            lon = 12.4964d;
            strcpy(selectedLocation, "Rome Italy");
            doDST = true;
            whereDST = CentralEurope;
            break;
        case HamburgGermany:
            lat = 53.5511d; 
            lon =  9.9937d;
            strcpy(selectedLocation, "Hamburg Germany");
            doDST = true;
            whereDST = CentralEurope;
            break;
        case AlertCanada:
            lat = 82.5018d; 
            lon = -62.34811d;
            strcpy(selectedLocation, "Alert Canada");
            sameasET = true; // Same time zone as ET
            doDST = true;
            whereDST = USA; // Same time zone as ET
            break;
        case Enteranylocation:
            // Set default info
            lat = 39.48333d; // Washington DC
            lon = -77.05d;
            rawtime = time(NULL);
            pts = gmtime(&rawtime);
            year    = pts->tm_year + 1900; // Current time and date
            month   = pts->tm_mon + 1;
            day     = pts->tm_mday;
            hours   = pts->tm_hour;
            minutes = pts->tm_min;
            seconds = pts->tm_sec;
            // Ask for position and date
            printf("Input latitude (+ is north) and Longitude (+ is east): ");
            fgets(buf, sizeof(buf), stdin);
            sscanf(buf, "%lf %lf", &lat, &lon);
            printf("Input date ( yyyy mm dd ) (ctrl-C exits): ");
            fgets(buf, sizeof(buf), stdin);
            sscanf(buf, "%d %d %d", &year, &month, &day);
            strcpy(selectedLocation, "?????");
            doDST = false;
            whereDST = noWhere;
            // Convert UTC time and date to local time and date
            (void) convertToLocal(&hours, &year, &month, &day, lon, true, doDST, whereDST, &UTCOffset);
            defaultLocation = true;
        break;
        default: // Washington DC
            lat = 39.48333d; 
            lon = -77.05d;
            strcpy(selectedLocation, "Washington DC");
            doDST = true;
            whereDST = USA;
            break;
        };
        
        system("cls"); // Clear the screen

        bool DST;
        // Get UTC (GMT) time and date
        if(!defaultLocation) {
            rawtime = time(NULL);
            pts = gmtime(&rawtime);
            year    = pts->tm_year + 1900;
            month   = pts->tm_mon + 1;
            day     = pts->tm_mday;
            hours   = pts->tm_hour;
            minutes = pts->tm_min;
            seconds = pts->tm_sec;

            if(bypassGMT) {
                // Convert UTC time and date to local time and date
                DST = convertToLocal(&hours, &year, &month, &day, 0.0d, true, doDST, whereDST, &UTCOffset);
                bypassGMT = false;
            }
            else if(addOffset) {
                hours += 1;
                if(hours > 12) day += 1;
                addOffset = false;
            }
            else if(sameasET) {
                DST = convertToLocal(&hours, &year, &month, &day, -77.05d, true, doDST,  whereDST, &UTCOffset);
                sameasET = false;
            }
            else {
                DST = convertToLocal(&hours, &year, &month, &day, lon, true, doDST, whereDST, &UTCOffset);					  
            }
        } // if(!defaultLocation)
        
        hours24 = hours;
        char AMPM[] = "am";
        if(hours >= 12) { // Convert to 12 hour format
            if(hours > 12) hours -= 12;
            strcpy(AMPM, "pm");
        }
        else if(hours == 0) {
            hours = 12;  
        }

        printf("\nThe location is %s (Latitude %5.5f, Longitude %5.5f)\n", selectedLocation, lat, lon);
        printf("The date is %s/%s/%d\t\t%s\n", convertIntToChar(monthS, month), convertIntToChar(dayS, day), year, getDayOfWeek(newDay, (dayOfWeek(year, month, day)) - 1));
        printf("The time is %s:%s:%s%s\t\t", convertIntToChar(hoursS, hours), convertIntToChar(minutesS, minutes), convertIntToChar(secondsS, seconds),
        AMPM);
        if(defaultLocation) {
            printf("\n");
            defaultLocation = false;
        }
        else if(doDST) {
            printf("%s\n", (DST) ? "Daylight Standard Time" : "Standard Time");
        }
        else {
            printf("%s %s\n", selectedLocation, "does not observe DST");
        }
        printf("UTC %s%d hours\t\t\t", (UTCOffset >= 0) ? "+" : "-", abs(UTCOffset));
        
        daylen  = day_length(year, month, day, lon, lat);
        civlen  = day_civil_twilight_length(year, month, day, lon, lat);
        nautlen = day_nautical_twilight_length(year, month, day, lon, lat);
        astrlen = day_astronomical_twilight_length(year, month, day, lon, lat);

        rs = sun_rise_set( year, month, day, lon, lat, &rise, &set );
        // Convert UTC sunrise time to local sunrise time
        int riseI = (int)round(rise);
        int setI = (int)round(set);
        (void) convertToLocal(&riseI, &year, &month, &day, lon, false, doDST, whereDST, &UTCOffset);
        // Convert UTC sunset time to local sunset time
        (void) convertToLocal(&setI, &year, &month, &day, lon, false, doDST, whereDST, &UTCOffset);

        // Determine night or day
        if(whereDST == noWhere) {
            printf("\n");
        }
        else {
            printf("%s%s\n\n", (hours24 >= setI ||  hours24 < riseI) ? "It is Night time in " : "It is Day time in ", selectedLocation);
        }      

#ifdef DISPLAYALL
        if(riseI >= 12) { // Convert to 12 hour format
            if(riseI > 12) riseI -= 12;
        }
        if(setI >= 12) { // Convert to 12 hour format
            if(setI > 12) setI -= 12;
        }
        
        
        civ = civil_twilight( year, month, day, lon, lat, &civ_start, &civ_end );
        // Convert UTC civ_start time to local civ_start time
        int civ_startI = (int)round(civ_start);
        int civ_endI = (int)round(civ_end);
        (void) convertToLocal(&civ_startI, &year, &month, &day, lon, false, doDST, whereDST, &UTCOffset);
        // Convert UTC civ_end time to local civ_end time
        (void) convertToLocal(&civ_endI, &year, &month, &day, lon, false, doDST, whereDST, &UTCOffset);
        if(civ_startI >= 12) { // Convert to 12 hour format
            if(civ_startI > 12) civ_startI -= 12;
        }
        if(civ_endI >= 12) { // Convert to 12 hour format
            if(civ_endI > 12) civ_endI -= 12;
        }

        naut = nautical_twilight( year, month, day, lon, lat, &naut_start, &naut_end );
        // Convert UTC naut_start time to local naut_start time
        int naut_startI = (int)round(naut_start);
        int naut_endI = (int)round(naut_end);
        (void) convertToLocal(&naut_startI, &year, &month, &day, lon, false, doDST, whereDST, &UTCOffset);
        // Convert UTC naut_end time to local naut_end time
        (void) convertToLocal(&naut_endI, &year, &month, &day, lon, false, doDST, whereDST, &UTCOffset);
        if(naut_startI >= 12) { // Convert to 12 hour format
            if(naut_startI > 12) naut_startI -= 12;
        }
        if(naut_endI >= 12) { // Convert to 12 hour format
            if(naut_endI > 12) naut_endI -= 12;
        }
        
        astr = astronomical_twilight( year, month, day, lon, lat, &astr_start, &astr_end );
        // Convert UTC astr_start time to local astr_start time
        int astr_startI = (int)round(astr_start);
        int astr_endI = (int)round(astr_end);
        (void) convertToLocal(&astr_startI, &year, &month, &day, lon, false, doDST, whereDST, &UTCOffset);
        // Convert UTC astr_end time to local astr_end time
        (void) convertToLocal(&astr_endI, &year, &month, &day, lon, false, doDST, whereDST, &UTCOffset);
        if(astr_startI >= 12) { // Convert to 12 hour format
            if(astr_startI > 12) astr_startI -= 12;
        }
        if(astr_endI >= 12) { // Convert to 12 hour format
            if(astr_endI > 12) astr_endI -= 12;
        }
        
//        printf("Sun at south %d hours UTC\n\n", (riseI+setI)/2);

        printf( "Day length:                 %5.2f hours\n", daylen );
        printf( "With civil twilight         %5.2f hours\n", civlen );
        printf( "With nautical twilight      %5.2f hours\n", nautlen );
        printf( "With astronomical twilight  %5.2f hours\n\n", astrlen );
        
        printf( "Length of twilight: civil   %5.2f hours\n", (civlen-daylen)/2.0);
        printf( "                  nautical  %5.2f hours\n", (nautlen-daylen)/2.0);
        printf( "              astronomical  %5.2f hours\n\n", (astrlen-daylen)/2.0);

        switch(rs)
        {
        case 0:
            printf( "Sun rises %dam, sets %dpm\n",
            riseI, setI );
            break;
        case +1:
            printf( "Sun above horizon\n" );
            break;
        case -1:
            printf( "Sun below horizon\n" );
            break;
        }

        switch(civ)
        {
        case 0:
            printf( "Civil twilight starts %dam, "
            "ends %dpm\n", civ_startI, civ_endI );
            break;
        case +1:
            printf( "Never darker than civil twilight\n" );
            break;
        case -1:
            printf( "Never as bright as civil twilight\n" );
            break;
        }

        switch(naut)
        {
        case 0:
            printf( "Nautical twilight starts %dam, "
            "ends %dpm\n", naut_startI, naut_endI );
            break;
        case +1:
            printf( "Never darker than nautical twilight\n" );
            break;
        case -1:
            printf( "Never as bright as nautical twilight\n" );
            break;
        }

        switch(astr)
        {
        case 0:
            printf( "Astronomical twilight starts %dam, "
            "ends %dpm\n", astr_startI, astr_endI );
            break;
        case +1:
            printf( "Never darker than astronomical twilight\n" );
            break;
        case -1:
            printf( "Never as bright as astronomical twilight\n" );
            break;
        }
#endif
        char inputAnswer[2];
        printf("\nAnother (Y/N)?: ");
        fgets(inputAnswer, sizeof(inputAnswer), stdin);
        sscanf(inputAnswer, "%s", &inputAnswer);
        if((strcmp(inputAnswer, "n")  == 0) || (strcmp(inputAnswer, "N") == 0)) {
            return 0;
        }
        system("cls"); // Clear the screen
    } // while (1)
}

/* The "workhorse" function for sun rise/set times */

int __sunriset__( int year, int month, int day, double lon, double lat,
double altit, int upper_limb, double *trise, double *tset )
/***************************************************************************/
/* Note: year,month,date = calendar date, 1801-2099 only.             */
/*       Eastern longitude positive, Western longitude negative       */
/*       Northern latitude positive, Southern latitude negative       */
/*       The longitude value IS critical in this function!            */
/*       altit = the altitude which the Sun should cross              */
/*               Set to -35/60 degrees for rise/set, -6 degrees       */
/*               for civil, -12 degrees for nautical and -18          */
/*               degrees for astronomical twilight.                   */
/*         upper_limb: non-zero -> upper limb, zero -> center         */
/*               Set to non-zero (e.g. 1) when computing rise/set     */
/*               times, and to zero when computing start/end of       */
/*               twilight.                                            */
/*        *rise = where to store the rise time                        */
/*        *set  = where to store the set  time                        */
/*                Both times are relative to the specified altitude,  */
/*                and thus this function can be used to compute       */
/*                various twilight times, as well as rise/set times   */
/* Return value:  0 = sun rises/sets this day, times stored at        */
/*                    *trise and *tset.                               */
/*               +1 = sun above the specified "horizon" 24 hours.     */
/*                    *trise set to time when the sun is at south,    */
/*                    minus 12 hours while *tset is set to the south  */
/*                    time plus 12 hours. "Day" length = 24 hours     */
/*               -1 = sun is below the specified "horizon" 24 hours   */
/*                    "Day" length = 0 hours, *trise and *tset are    */
/*                    both set to the time when the sun is at south.  */
/*                                                                    */
/**********************************************************************/
{
    double  d,  /* Days since 2000 Jan 0.0 (negative before) */
    sr,         /* Solar distance, astronomical units */
    sRA,        /* Sun's Right Ascension */
    sdec,       /* Sun's declination */
    sradius,    /* Sun's apparent radius */
    t,          /* Diurnal arc */
    tsouth,     /* Time when Sun is at south */
    sidtime;    /* Local sidereal time */

    int rc = 0; /* Return code from function - usually 0 */

    /* Compute d of 12h local mean solar time */
    d = days_since_2000_Jan_0(year,month,day) + 0.5 - lon/360.0;

    /* Compute the local sidereal time of this moment */
    sidtime = revolution( GMST0(d) + 180.0 + lon );

    /* Compute Sun's RA, Decl and distance at this moment */
    sun_RA_dec( d, &sRA, &sdec, &sr );

    /* Compute time when Sun is at south - in hours UT */
    tsouth = 12.0 - rev180(sidtime - sRA)/15.0;

    /* Compute the Sun's apparent radius in degrees */
    sradius = 0.2666 / sr;

    /* Do correction to upper limb, if necessary */
    if( upper_limb )
    altit -= sradius;

    /* Compute the diurnal arc that the Sun traverses to reach */
    /* the specified altitude altit: */
    {
        double cost;
        cost = ( sind(altit) - sind(lat) * sind(sdec) ) /
        ( cosd(lat) * cosd(sdec) );
        if( cost >= 1.0 )
        rc = -1, t = 0.0;       /* Sun always below altit */
        else if( cost <= -1.0 )
        rc = +1, t = 12.0;      /* Sun always above altit */
        else
        t = acosd(cost)/15.0;   /* The diurnal arc, hours */
    }

    /* Store rise and set times - in hours UT */
    *trise = tsouth - t;
    *tset  = tsouth + t;

    return rc;
}  /* __sunriset__ */



/* The "workhorse" function */


double __daylen__( int year, int month, int day, double lon, double lat,
double altit, int upper_limb )
/**********************************************************************/
/* Note: year,month,date = calendar date, 1801-2099 only.             */
/*       Eastern longitude positive, Western longitude negative       */
/*       Northern latitude positive, Southern latitude negative       */
/*       The longitude value is not critical. Set it to the correct   */
/*       longitude if you're picky, otherwise set to to, say, 0.0     */
/*       The latitude however IS critical - be sure to get it correct */
/*       altit = the altitude which the Sun should cross              */
/*               Set to -35/60 degrees for rise/set, -6 degrees       */
/*               for civil, -12 degrees for nautical and -18          */
/*               degrees for astronomical twilight.                   */
/*         upper_limb: non-zero -> upper limb, zero -> center         */
/*               Set to non-zero (e.g. 1) when computing day length   */
/*               and to zero when computing day+twilight length.      */
/**********************************************************************/
{
    double  d,  /* Days since 2000 Jan 0.0 (negative before) */
    obl_ecl,    /* Obliquity (inclination) of Earth's axis */
    sr,         /* Solar distance, astronomical units */
    slon,       /* True solar longitude */
    sin_sdecl,  /* Sine of Sun's declination */
    cos_sdecl,  /* Cosine of Sun's declination */
    sradius,    /* Sun's apparent radius */
    t;          /* Diurnal arc */

    /* Compute d of 12h local mean solar time */
    d = days_since_2000_Jan_0(year,month,day) + 0.5 - lon/360.0;

    /* Compute obliquity of ecliptic (inclination of Earth's axis) */
    obl_ecl = 23.4393 - 3.563E-7 * d;

    /* Compute Sun's ecliptic longitude and distance */
    sunpos( d, &slon, &sr );

    /* Compute sine and cosine of Sun's declination */
    sin_sdecl = sind(obl_ecl) * sind(slon);
    cos_sdecl = sqrt( 1.0 - sin_sdecl * sin_sdecl );

    /* Compute the Sun's apparent radius, degrees */
    sradius = 0.2666 / sr;

    /* Do correction to upper limb, if necessary */
    if( upper_limb )
    altit -= sradius;

    /* Compute the diurnal arc that the Sun traverses to reach */
    /* the specified altitude altit: */
    {
        double cost;
        cost = ( sind(altit) - sind(lat) * sin_sdecl ) /
        ( cosd(lat) * cos_sdecl );
        if( cost >= 1.0 )
        t = 0.0;                      /* Sun always below altit */
        else if( cost <= -1.0 )
        t = 24.0;                     /* Sun always above altit */
        else  t = (2.0/15.0) * acosd(cost); /* The diurnal arc, hours */
    }
    return t;
}  /* __daylen__ */


/* This function computes the Sun's position at any instant */

void sunpos( double d, double *lon, double *r )
/******************************************************/
/* Computes the Sun's ecliptic longitude and distance */
/* at an instant given in d, number of days since     */
/* 2000 Jan 0.0.  The Sun's ecliptic latitude is not  */
/* computed, since it's always very near 0.           */
/******************************************************/
{
    double M,         /* Mean anomaly of the Sun */
    w,         /* Mean longitude of perihelion */
    /* Note: Sun's mean longitude = M + w */
    e,         /* Eccentricity of Earth's orbit */
    E,         /* Eccentric anomaly */
    x, y,      /* x, y coordinates in orbit */
    v;         /* True anomaly */

    /* Compute mean elements */
    M = revolution( 356.0470 + 0.9856002585 * d );
    w = 282.9404 + 4.70935E-5 * d;
    e = 0.016709 - 1.151E-9 * d;

    /* Compute true longitude and radius vector */
    E = M + e * RADEG * sind(M) * ( 1.0 + e * cosd(M) );
    x = cosd(E) - e;
    y = sqrt( 1.0 - e*e ) * sind(E);
    *r = sqrt( x*x + y*y );              /* Solar distance */
    v = atan2d( y, x );                  /* True anomaly */
    *lon = v + w;                        /* True solar longitude */
    if( *lon >= 360.0 )
    *lon -= 360.0;                   /* Make it 0..360 degrees */
}

void sun_RA_dec( double d, double *RA, double *dec, double *r )
/******************************************************/
/* Computes the Sun's equatorial coordinates RA, Decl */
/* and also its distance, at an instant given in d,   */
/* the number of days since 2000 Jan 0.0.             */
/******************************************************/
{
    double lon, obl_ecl, x, y, z;

    /* Compute Sun's ecliptical coordinates */
    sunpos( d, &lon, r );

    /* Compute ecliptic rectangular coordinates (z=0) */
    x = *r * cosd(lon);
    y = *r * sind(lon);

    /* Compute obliquity of ecliptic (inclination of Earth's axis) */
    obl_ecl = 23.4393 - 3.563E-7 * d;

    /* Convert to equatorial rectangular coordinates - x is unchanged */
    z = y * sind(obl_ecl);
    y = y * cosd(obl_ecl);

    /* Convert to spherical coordinates */
    *RA = atan2d( y, x );
    *dec = atan2d( z, sqrt(x*x + y*y) );

}  /* sun_RA_dec */


/******************************************************************/
/* This function reduces any angle to within the first revolution */
/* by subtracting or adding even multiples of 360.0 until the     */
/* result is >= 0.0 and < 360.0                                   */
/******************************************************************/

#define INV360    ( 1.0 / 360.0 )

double revolution( double x )
/*****************************************/
/* Reduce angle to within 0..360 degrees */
/*****************************************/
{
    return( x - 360.0 * floor( x * INV360 ) );
}  /* revolution */

double rev180( double x )
/*********************************************/
/* Reduce angle to within +180..+180 degrees */
/*********************************************/
{
    return( x - 360.0 * floor( x * INV360 + 0.5 ) );
}  /* revolution */


/*******************************************************************/
/* This function computes GMST0, the Greenwich Mean Sidereal Time  */
/* at 0h UT (i.e. the sidereal time at the Greenwhich meridian at  */
/* 0h UT).  GMST is then the sidereal time at Greenwich at any     */
/* time of the day.  I've generalized GMST0 as well, and define it */
/* as:  GMST0 = GMST - UT  --  this allows GMST0 to be computed at */
/* other times than 0h UT as well.  While this sounds somewhat     */
/* contradictory, it is very practical:  instead of computing      */
/* GMST like:                                                      */
/*                                                                 */
/*  GMST = (GMST0) + UT * (366.2422/365.2422)                      */
/*                                                                 */
/* where (GMST0) is the GMST last time UT was 0 hours, one simply  */
/* computes:                                                       */
/*                                                                 */
/*  GMST = GMST0 + UT                                              */
/*                                                                 */
/* where GMST0 is the GMST "at 0h UT" but at the current moment!   */
/* Defined in this way, GMST0 will increase with about 4 min a     */
/* day.  It also happens that GMST0 (in degrees, 1 hr = 15 degr)   */
/* is equal to the Sun's mean longitude plus/minus 180 degrees!    */
/* (if we neglect aberration, which amounts to 20 seconds of arc   */
/* or 1.33 seconds of time)                                        */
/*                                                                 */
/*******************************************************************/

double GMST0( double d )
{
    double sidtim0;
    /* Sidtime at 0h UT = L (Sun's mean longitude) + 180.0 degr  */
    /* L = M + w, as defined in sunpos().  Since I'm too lazy to */
    /* add these numbers, I'll let the C compiler do it for me.  */
    /* Any decent C compiler will add the constants at compile   */
    /* time, imposing no runtime or code overhead.               */
    sidtim0 = revolution( ( 180.0 + 356.0470 + 282.9404 ) +
    ( 0.9856002585 + 4.70935E-5 ) * d );
    return sidtim0;
}  /* GMST0 */
/* Helper functions for determing local time & date (from UTC)

The following three functions ripped off from Electrical Engineering Stack Exchange
http://electronics.stackexchange.com/questions/66285/how-to-calculate-day-of-the-week-for-rtc

Returns the number of days to the start of the specified year, taking leap
years into account, but not the shift from the Julian calendar to the
Gregorian calendar. Instead, it is as though the Gregorian calendar is
extrapolated back in time to a hypothetical "year zero".
*/
unsigned int leap(unsigned int year)
{
    return year * 365 + (year / 4) - (year / 100) + (year / 400);
}
/* Returns a number representing the number of days since March 1 in the
hypothetical year 0, not counting the change from the Julian calendar
to the Gregorian calendar that occurred in the 16th century. This
algorithm is loosely based on a function known as "Zeller's Congruence".
This number MOD 7 gives the day of week, where 0 = Monday and 6 = Sunday.
*/
unsigned int zeller(unsigned int year, unsigned int month, unsigned int day)
{
    year += ((month + 9) / 12) - 1;
    month = (month + 9) % 12;
    return leap(year) + month * 30 + ((6 * month + 5) / 10) + day + 1;
}

// Returns the day of week for a given date.
unsigned int dayOfWeek(unsigned int year, unsigned int month, unsigned int day)
{
    return (zeller(year, month, day) % 7) + 1;
}

/* Ripped off from Stackoverflow
http://stackoverflow.com/questions/5590429/calculating-daylight-saving-time-from-only-date

Check to see if it's Daylight Savings Time (DST)
*/
bool IsDST(unsigned int day, unsigned int month , unsigned int DOW)
{
    // Make Day of Week (DOW) match with what Stackoverflow suggests
    // for DOW (Sunday = 0 to Saturday = 6)
    switch (DOW)
    {
    case 6:  DOW = 0; break; // Sun
    case 7:  DOW = 1; break; // Mon
    case 1:  DOW = 2; break; // Tue
    case 2:  DOW = 3; break; // Wed
    case 3:  DOW = 4; break; // Thu
    case 4:  DOW = 5; break; // Fri
    case 5:  DOW = 6; break; // Sat
    default: break;
    }
    // January, February, and December are out
    if(month < 3 || month > 11) {
        return false;
    }
    // April to October are in
    if(month > 3 && month < 11) {
        return true;
    }
    int previousSunday = (int)(day - DOW);
    // In march, we are DST if our previous Sunday was on or after the 8th
    if(month == 3) {
        return previousSunday >= 8;
    }
    // In November we must be before the first Sunday to be DST
    // That means the previous Sunday must be before the 1st
    return previousSunday <= 0;
}

/* Convert UTC time and date to local time and date
Difference between UTC time/date (at Greenwich) and local time/date is 15 minutes
per 1 degree of longitude. See the following:
http://www.edaboard.com/thread101516.html
*/
bool convertToLocal(int* hour, int* year, int* month,
int* day, double lon, bool convertDate,
bool doDST, whereDST_t whereDST, int* pUTCOffset) {

    int UTCOffset = 0;
    int tempUTCOffset = 0;
    bool DST = false;
    unsigned int DaysAMonth[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

    // Get Day of Week (DOW)
    unsigned int DOW = dayOfWeek(*year, *month, *day);
    switch(whereDST)
    {
    case USA:
        // Get Daylight Saving Time (DST) or Standard Time (ST) for USA
        DST = IsDST(*day, *month, DOW);
        break;
    case CentralEurope:
        // Get Daylight Saving Time (DST) or Standard Time (ST) for Central Europe
        DST = IsDSTCentralEurope(*day, *month, DOW);
        break;
    case Asia:
        // Not yet implemented
        DST = false;
        break;
    case Australia:
        // Not yet implemented
        DST = false;
        break;
    case someWhereElse:
        // Not yet implemented
        DST = false;
        break;
    case noWhere:
        // No idea if it's DST
        DST = false;
        break;
    default:
        DST = false;
    }
    if(abs(lon) <= 118.0d) {
        UTCOffset = (int)round((lon / 15.0d)); // UTC offset
    }
    else {
        UTCOffset = (int)(((int)lon / 15)); // UTC offset 
    }
    tempUTCOffset = UTCOffset;
    if(UTCOffset < 0) {
        // West of Greenwich, subtract
        UTCOffset = abs(UTCOffset); // Make offset positive
        if(DST && convertDate && doDST) --UTCOffset; // Compensate for DST
        if(*hour <= UTCOffset) *hour += 24;
        *hour -= UTCOffset; // Subtract offset
    }
    else if(UTCOffset > 0) {
        // East of Greenwich, add
        if(DST && convertDate && doDST) --UTCOffset; // Compensate for DST
        *hour += UTCOffset; // Add offset
        if(*hour >= 24) {
            *hour -= 24;
            if(convertDate) *day += 1;
        }
        else if((24 - *hour) <= UTCOffset) {
            if(convertDate) *day += 1;
        }
    }
    *pUTCOffset = tempUTCOffset;

    // Convert date if convertDate flag is set
    // Portions of the following code (with some modifications) ripped off from Instructables
    // http://www.instructables.com/id/GPS-time-UTC-to-local-time-conversion-using-Arduin/?ALLSTEPS
    if(convertDate) {
        if((24 - *hour) <= UTCOffset) { // A new UTC day started
            if(*year % 4 == 0) DaysAMonth[1] = 29; //leap year check (the simple method)
            if(*hour < 24) {
                *day -= 1;
                if(*day < 1) {
                    if(*month == 1) {
                        *month = 12;
                        *year -= 1;
                    } // if(*month == 1)
                    else {
                        *month -= 1;
                    }
                    *day = DaysAMonth[*month - 1];
                } // if(*day < 1)
            } // if(*hour < 24)
            else if(*hour >= 24) {
                *day += 1;
                if(*day > DaysAMonth[*month - 1]) {
                    *day = 1;
                    *month += 1;
                    if(*month > 12) *year += 1;
                } // if(*day > DaysAMonth[*month - 1])
            } // if(*hour >= 24)
        } // if((24 - *hour) <= UTCOffset)
    } // if(convertDate)
    return (DST);
}

char* convertIntToChar(char* inChar, int toConvert) {
    sprintf(inChar, "%d", toConvert);
    if(toConvert < 10) {
        sprintf(inChar, "0%d", toConvert);
    }
    return (inChar);
}

// Get the day of the week
char* getDayOfWeek(char* inChar, int day)
{
    if(day == 7) day = 0;
    switch (day)
    {
    case 0:   strcpy(inChar, "Monday"); break;
    case 1:   strcpy(inChar, "Tuesday"); break;
    case 2:   strcpy(inChar, "Wednesday"); break;
    case 3:   strcpy(inChar, "Thursday"); break;
    case 4:   strcpy(inChar, "Friday"); break;
    case 5:   strcpy(inChar, "Saturday"); break;
    case 6:   strcpy(inChar, "Sunday"); break;
    default:  strcpy(inChar, "Error"); break;
    }
    return (inChar);
}

#if 1
bool IsDSTCentralEurope(int day, int month, int DOW)
{
    if(month < 3 || month > 10)  return false; 
    if(month > 3 && month < 10)  return true; 

    int previousSunday = day - DOW;

    if(month == 3) return previousSunday >= 25;
    if(month == 10) return previousSunday < 25;

    return false; // this line never gonna happend
}
#endif