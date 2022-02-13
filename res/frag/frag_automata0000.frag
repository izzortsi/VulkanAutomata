//	----    ----    ----    ----    ----    ----    ----    ----
//  Shader developed by Slackermanz
//
//  Info/Code:
//  ﻿ - Website: https://slackermanz.com
//  ﻿ - Github: https://github.com/Slackermanz
//  ﻿ - Shadertoy: https://www.shadertoy.com/user/SlackermanzCA
//  ﻿ - Discord: https://discord.gg/hqRzg74kKT
//  
//  Socials:
//  ﻿ - Discord DM: Slackermanz#3405
//  ﻿ - Reddit DM: https://old.reddit.com/user/slackermanz
//  ﻿ - Twitter: https://twitter.com/slackermanz
//  ﻿ - YouTube: https://www.youtube.com/c/slackermanz
//  ﻿ - Older YT: https://www.youtube.com/channel/UCZD4RoffXIDoEARW5aGkEbg
//  ﻿ - TikTok: https://www.tiktok.com/@slackermanz
//  
//  Communities:
//  ﻿ - Reddit: https://old.reddit.com/r/cellular_automata
//  ﻿ - Artificial Life: https://discord.gg/7qvBBVca7u
//  ﻿ - Emergence: https://discord.com/invite/J3phjtD
//  ﻿ - ConwayLifeLounge: https://discord.gg/BCuYCEn
//	----    ----    ----    ----    ----    ----    ----    ----

#version 460
#define PI 3.14159265359
#define LN 2.71828182846

//	----    ----    ----    ----    ----    ----    ----    ----

layout(location 	=  0) out 		vec4 		out_col;
layout(binding 		=  1) uniform 	sampler2D 	txdata;
layout(binding 		=  0) uniform 	UniBuf {
	uint v0;  uint v1;  uint v2;  uint v3;	uint v4;  uint v5;  uint v6;  uint v7;
	uint v8;  uint v9;  uint v10; uint v11;	uint v12; uint v13; uint v14; uint v15;
	uint v16; uint v17; uint v18; uint v19;	uint v20; uint v21; uint v22; uint v23;
	uint v24; uint v25; uint v26; uint v27;	uint v28; uint v29; uint v30; uint v31;
	uint v32; uint v33; uint v34; uint v35;	uint v36; uint v37; uint v38; uint v39;
	uint v40; uint v41; uint v42; uint v43;	uint v44; uint v45; uint v46; uint v47;
	uint v48; uint v49; uint v50; uint v51;	uint v52; uint v53; uint v54; uint v55;
	uint v56; uint v57; uint v58; uint v59;	uint v60; uint v61; uint v62; uint v63; } ub;

//	----    ----    ----    ----    ----    ----    ----    ----

const uint MAX_RADIUS = 12u;

//	----    ----    ----    ----    ----    ----    ----    ----

struct ConvData {
	vec4 	value;
	float 	total;
};

struct RingData {
	ConvData[MAX_RADIUS] range;
};

uint u32_upk(uint u32, uint bts, uint off) { return (u32 >> off) & ((1u << bts)-1u); }

float lmap() { return (gl_FragCoord[0] / textureSize(txdata,0)[0]); }
float vmap() { return (gl_FragCoord[1] / textureSize(txdata,0)[1]); }
float cmap() { return sqrt	( ((gl_FragCoord[0] - textureSize(txdata,0)[0]*0.5) / textureSize(txdata,0)[0]*0.5)
							* ((gl_FragCoord[0] - textureSize(txdata,0)[0]*0.5) / textureSize(txdata,0)[0]*0.5)
							+ ((gl_FragCoord[1] - textureSize(txdata,0)[1]*0.5) / textureSize(txdata,0)[1]*0.5)
							* ((gl_FragCoord[1] - textureSize(txdata,0)[1]*0.5) / textureSize(txdata,0)[1]*0.5) ); }

float vwm() {
	float 	scale_raw 	= uintBitsToFloat(ub.v62);
	float 	zoom 		= uintBitsToFloat(ub.v61);
	float	scale_new	= scale_raw;
	uint 	mode 		= u32_upk(ub.v59, 2u, 0u);
	if( mode == 1u ) { //	Linear Parameter Map
		scale_new = ((lmap() + zoom) * (scale_raw / (1.0 + zoom * 2.0))) * 2.0; }
	if( mode == 2u ) { //	Circular Parameter Map
		scale_new = ((sqrt(cmap()) + zoom) * (scale_raw / (1.0 + zoom * 2.0))) * 2.0; }
	return scale_new; }

float  tp(uint n, float s) 			{ return (float(n+1u)/256.0) * ((s*0.5)/128.0); }
float bsn(uint v, uint  o) 			{ return float(u32_upk(v,1u,o)*2u)-1.0; }
float utp(uint v, uint  w, uint o) 	{ return tp(u32_upk(v,w,w*o), vwm()); }

vec4  sigm(vec4  x, float w) { return 1.0 / ( 1.0 + exp( (-w*2.0 * x * (PI/2.0)) + w * (PI/2.0) ) ); }
float hmp2(float x, float w) { return 3.0*((x-0.5)*(x-0.5))+0.25; }

vec4  gdv( ivec2 of, sampler2D tx ) {
	of 		= ivec2(gl_FragCoord) + of;
	of[0] 	= (of[0] + textureSize(tx,0)[0]) & (textureSize(tx,0)[0]-1);
	of[1] 	= (of[1] + textureSize(tx,0)[1]) & (textureSize(tx,0)[1]-1);
	return 	texelFetch( tx, of, 0); }


ConvData ring( float r ) {

	const float psn = 32768.0;

	float tot = 0.0;
	vec4  val = vec4(0.0,0.0,0.0,0.0);

	float sq2	= sqrt(2.0);

	float o_0 = r + 0.5;
	float o_1 = sq2 * o_0;
	float o_2 = o_1 / 2.0;
	float o_3 = sqrt( o_0*o_0 - r*r );
	float o_4 = o_2 - ( floor(o_2) + 0.5 );
	float o_5 = floor( o_2 ) + floor( o_4 );

	float i_0 = r - 0.5;
	float i_1 = sq2 * i_0;
	float i_2 = i_1 / 2.0;
	float i_3 = sqrt( i_0*i_0 - r*r );
	float i_4 = i_2 - ( floor(i_2) + 1.0 );
	float i_5 = floor( i_2 ) + floor( i_4 );

	float d_0 = ( i_5 ) + 1.0 - ( o_5 );

	for(float i = 1.0; i < floor( i_2 ) + 1.0 - d_0; i++) {

		float j_0 = sqrt( o_0*o_0 - (i+0.0)*(i+0.0) );
		float j_1 = sqrt( i_0*i_0 - (i+0.0)*(i+0.0) );
		float j_2 = ( 1.0 - abs( sign ( (floor( i_2 ) + 1.0) - i ) ) );

		for(float j = floor( j_1 ) + j_2; j < floor( j_0 ); j++) {
			val += floor(gdv(ivec2( i, (j+1)), txdata) * psn);
			val += floor(gdv(ivec2( i,-(j+1)), txdata) * psn);
			val += floor(gdv(ivec2(-i,-(j+1)), txdata) * psn);
			val += floor(gdv(ivec2(-i, (j+1)), txdata) * psn);
			val += floor(gdv(ivec2( (j+1), i), txdata) * psn);
			val += floor(gdv(ivec2( (j+1),-i), txdata) * psn);
			val += floor(gdv(ivec2(-(j+1),-i), txdata) * psn);
			val += floor(gdv(ivec2(-(j+1), i), txdata) * psn);
			tot += 8.0 * psn; } }

//	Orthagonal
	val += floor(gdv(ivec2( r, 0), txdata) * psn);
	val += floor(gdv(ivec2( 0,-r), txdata) * psn);
	val += floor(gdv(ivec2(-r,-0), txdata) * psn);
	val += floor(gdv(ivec2(-0, r), txdata) * psn);
	tot += 4.0 * psn;

//	Diagonal
//	TODO This is not quite perfect
	float k_0 = r;
	float k_1 = sq2 * k_0;
	float k_2 = k_1 / 2.0;
	float k_3 = sqrt( k_0*k_0 - r*r );
	float k_4 = k_2 - ( floor(k_2) + 1.0 );
	float k_5 = floor( k_2 ) + floor( k_4 );

	float dist = round(k_2);

	if( sign( o_4 ) == -1.0 ) {
	//	val += gdv(ivec2( (floor(o_5)+1), floor(o_5)+1), txdata);
		val += floor(gdv(ivec2( (floor(o_5)+1), (floor(o_5)+1)), txdata) * psn);
		val += floor(gdv(ivec2( (floor(o_5)+1),-(floor(o_5)+1)), txdata) * psn);
		val += floor(gdv(ivec2(-(floor(o_5)+1),-(floor(o_5)+1)), txdata) * psn);
		val += floor(gdv(ivec2(-(floor(o_5)+1), (floor(o_5)+1)), txdata) * psn);
		tot += 4.0 * psn; }

	return ConvData( val, tot ); }

vec4[2] nbhd3( vec2 r, sampler2D tx ) {
//	Precision limit of signed float32 for [n] neighbors in a 16 bit texture (symmetry preservation)
	uint	chk = 2147483648u /
			(	( 	uint( r[0]*r[0]*PI + r[0]*PI + PI + 1u	)
				- 	uint( r[1]*r[1]*PI + r[1]*PI			) ) * 128u );
	float	psn = (chk >= 65536u) ? 65536.0 : float(chk);
	vec4	a = vec4(0.0,0.0,0.0,0.0);
	vec4 	b = vec4(0.0,0.0,0.0,0.0);
	float	w = 1.0;	// Weighting, unused
	if(r[0] == 0.0) { return vec4[2]( gdv( ivec2(0,0), tx )*w*psn, vec4(psn,psn,psn,psn) ); }
	else             {
		vec2 r2 = ceil(r + vec2(0.5)) - vec2(0.500001);
		r2 = r2 * r2;
		for(float j = 0.0; j <= r[0]; j++) {
		    vec2 bound = sqrt(max(vec2(0),r2 - vec2(j*j)));
		    for(float i = floor(bound[1])+1; i <= bound[0]; i++) {
		        w  = 1.0;    //    Per-Neighbor Weighting, unused
		        b += w * psn * 4.0;
		        vec4 t0  = gdv( ivec2( i, j), tx ) * w * psn; a += t0 - fract(t0);
		        vec4 t1  = gdv( ivec2( j,-i), tx ) * w * psn; a += t1 - fract(t1);
		        vec4 t2  = gdv( ivec2(-i,-j), tx ) * w * psn; a += t2 - fract(t2);
		        vec4 t3  = gdv( ivec2(-j, i), tx ) * w * psn; a += t3 - fract(t3); } } 
		return vec4[2](a, b); } }

vec4[2] nbhd1( vec2 r, sampler2D tx ) {
//	Precision limit of signed float32 for [n] neighbors in a 16 bit texture (symmetry preservation)
	uint	chk = 2147483648u /
			(	( 	uint( r[0]*r[0]*PI + r[0]*PI + PI + 1u	)
				- 	uint( r[1]*r[1]*PI + r[1]*PI			) ) * 128u );
	float	psn = (chk >= 65536u) ? 65536.0 : float(chk);
	vec4	a = vec4(0.0,0.0,0.0,0.0);
	vec4 	b = vec4(0.0,0.0,0.0,0.0);
	float	w = 1.0;	// Weighting, unused
	if(r[0] == 0.0) { return vec4[2]( gdv( ivec2(0,0), tx )*w*psn, vec4(psn,psn,psn,psn) ); }
	else 			{
		for(float i = 0.0; i <= r[0]; i++) {
			for(float j = 1.0; j <= r[0]; j++) {
				float	d = round(sqrt(i*i+j*j));
						w = 1.0;	//	Per-Neighbor Weighting, unused
				if( d <= r[0] && d > r[1] ) {
						 b 	+= w * psn * 4.0;
					vec4 t0  = gdv( ivec2( i, j), tx ) * w * psn; a += t0 - fract(t0);
					vec4 t1  = gdv( ivec2( j,-i), tx ) * w * psn; a += t1 - fract(t1);
					vec4 t2  = gdv( ivec2(-i,-j), tx ) * w * psn; a += t2 - fract(t2);
					vec4 t3  = gdv( ivec2(-j, i), tx ) * w * psn; a += t3 - fract(t3); } } }
		return vec4[2](a, b); } }

vec4[2] nbhd2( vec2 r, sampler2D tx ) {
//	Precision limit of signed float32 for [n] neighbors in a 16 bit texture (symmetry preservation)
	uint	chk = 2147483648u /
			(	( 	uint( r[0]*r[0]*PI + r[0]*PI + PI + 1u	)
				- 	uint( r[1]*r[1]*PI + r[1]*PI			) ) * 128u );
	float	psn = (chk >= 65536u) ? 65536.0 : float(chk);
	vec4	a = vec4(0.0,0.0,0.0,0.0);
	vec4 	b = vec4(0.0,0.0,0.0,0.0);
	float	w = 1.0;	// Weighting, unused
	if(r[0] == 0.0) { return vec4[2]( gdv( ivec2(0,0), tx )*w*psn, vec4(psn,psn,psn,psn) ); }
		else             {
		    vec2 r2 = r * r;
		    for(float i = 0.0; i <= r[0]; i++) {
		        vec2 bound = sqrt(r2 - vec2(i*i));
		        for(float j = floor(bound[1]) + 1; j <= bound[0]; j++) {
		            w  = 1.0;    //    Per-Neighbor Weighting, unused
		            b += w * psn * 4.0;
		            vec4 t0  = gdv( ivec2( i, j), tx ) * w * psn; a += t0 - fract(t0);
		            vec4 t1  = gdv( ivec2( j,-i), tx ) * w * psn; a += t1 - fract(t1);
		            vec4 t2  = gdv( ivec2(-i,-j), tx ) * w * psn; a += t2 - fract(t2);
		            vec4 t3  = gdv( ivec2(-j, i), tx ) * w * psn; a += t3 - fract(t3); } } 
		    return vec4[2](a, b); } }

vec4 bitring(vec4[MAX_RADIUS][2] rings, uint bits, uint of) {
	vec4 sum = vec4(0.0,0.0,0.0,0.0);
	vec4 tot = vec4(0.0,0.0,0.0,0.0);
	for(uint i = 0u; i < MAX_RADIUS; i++) {
		if(u32_upk(bits, 1u, i+of) == 1u) { sum += rings[i][0]; tot += rings[i][1]; } }
/**/	return sum / tot; }	/*/
	return sigm( (sum / tot), LN ); } /**/// TODO

vec4 bitmake(ConvData[MAX_RADIUS] rings, uint bits, uint of) {
	vec4  sum = vec4(0.0,0.0,0.0,0.0);
	float tot = 0.0;
	for(uint i = 0u; i < MAX_RADIUS; i++) {
		if(u32_upk(bits, 1u, i+of) == 1u) { sum += rings[i].value; tot += rings[i].total; } }
/**/	return sum / tot; }	/*/
	return sigm( (sum / tot), LN ); } /**/// TODO

//	----    ----    ----    ----    ----    ----    ----    ----

//	Used to reseed the surface with lumpy noise
//	TODO - Breaks down at 2048+ resolution
float get_xc(float x, float y, float xmod) {
	float sq = sqrt(mod(x*y+y, xmod)) / sqrt(xmod);
	float xc = mod((x*x)+(y*y), xmod) / xmod;
	return clamp((sq+xc)*0.5, 0.0, 1.0); }
float shuffle(float x, float y, float xmod, float val) {
	val = val * mod( x*y + x, xmod );
	return (val-floor(val)); }
float get_xcn(float x, float y, float xm0, float xm1, float ox, float oy) {
	float  xc = get_xc(x+ox, y+oy, xm0);
	return shuffle(x+ox, y+oy, xm1, xc); }
float get_lump(float x, float y, float nhsz, float xm0, float xm1) {
	float 	nhsz_c 	= 0.0;
	float 	xcn 	= 0.0;
	float 	nh_val 	= 0.0;
	for(float i = -nhsz; i <= nhsz; i += 1.0) {
		for(float j = -nhsz; j <= nhsz; j += 1.0) {
			nh_val = round(sqrt(i*i+j*j));
			if(nh_val <= nhsz) {
				xcn = xcn + get_xcn(x, y, xm0, xm1, i, j);
				nhsz_c = nhsz_c + 1.0; } } }
	float 	xcnf 	= ( xcn / nhsz_c );
	float 	xcaf	= xcnf;
	for(float i = 0.0; i <= nhsz; i += 1.0) {
			xcaf 	= clamp((xcnf*xcaf + xcnf*xcaf) * (xcnf+xcnf), 0.0, 1.0); }
	return xcaf; }
float reseed(uint seed, float scl, float amp) {
	float 	fx = gl_FragCoord[0];
	float 	fy = gl_FragCoord[1];
	float 	r0 = get_lump(fx, fy, round( 6.0  * scl), 19.0 + mod(u32_upk(ub.v63, 24u, 0u)+seed,17.0), 23.0 + mod(u32_upk(ub.v63, 24u, 0u)+seed,43.0));
	float 	r1 = get_lump(fx, fy, round( 22.0 * scl), 13.0 + mod(u32_upk(ub.v63, 24u, 0u)+seed,29.0), 17.0 + mod(u32_upk(ub.v63, 24u, 0u)+seed,31.0));
	float 	r2 = get_lump(fx, fy, round( 14.0 * scl), 13.0 + mod(u32_upk(ub.v63, 24u, 0u)+seed,11.0), 51.0 + mod(u32_upk(ub.v63, 24u, 0u)+seed,37.0));
	float 	r3 = get_lump(fx, fy, round( 18.0 * scl), 29.0 + mod(u32_upk(ub.v63, 24u, 0u)+seed, 7.0), 61.0 + mod(u32_upk(ub.v63, 24u, 0u)+seed,28.0));
	return clamp( sqrt((r0+r1)*r3*(amp+1.2))-r2*(amp*1.8+0.2) , 0.0, 1.0); }

vec4 place( vec4 col, float sz, vec2 mxy, uint s, float off ) {
	vec2 dxy = (vec2(gl_FragCoord) - mxy) * (vec2(gl_FragCoord) - mxy);
	float dist = sqrt(dxy[0] + dxy[1]);
	float cy = mod(u32_upk(ub.v63, 24u, 0u)+off, 213.0) / 213.0;
	float c2 = mod(u32_upk(ub.v63, 24u, 0u)+off, 377.0) / 377.0;
	float z2 = ((cos(2.0*PI*c2)/2.0)+0.5);
	float z3 = z2/4.0;
	float z4 = z2-z3;
	float ds = (1.0-dist/sz);
	float vr = (((cos((1.0*PI*4.0*cy)/2.0)+0.5) * z4 + z3) * ds * 0.85 + 0.38 * ds * ds);
	float vg = (((cos((2.0*PI*4.0*cy)/2.0)+0.5) * z4 + z3) * ds * 0.85 + 0.38 * ds * ds);
	float vb = (((cos((3.0*PI*4.0*cy)/2.0)+0.5) * z4 + z3) * ds * 0.85 + 0.38 * ds * ds);
	if(dist <= sz) { col += (s != 1u) ? vec4(-0.38,-0.38,-0.38,-0.38)*ds : vec4(vr,vg,vb,1.0); }
	return col; }

vec4 mouse(vec4 col, float sz) {
	vec2 mxy = vec2( u32_upk(ub.v60, 12u, 0u), u32_upk(ub.v60, 12u, 12u) );
	return place(col, sz, mxy, u32_upk(ub.v60, 2u, 24u), 0.0); }

vec4 symsd(vec4 col, float sz) {
	vec2 posxy = vec2(textureSize(txdata,0)[0]/2.0,textureSize(txdata,0)[1]/2.0);
	for(int i = 0; i < 11; i++) {
		uint sn = ((i&2u)==0u) ? 1u : 0u;
		col = place(col, (sz/11.0)*((11.0-i)), 		posxy + vec2(  0.0, 0.0 ), sn, i*u32_upk(ub.v63, 24u, 0u));
		col = place(col, (sz/11.0)*((11.0-i))*0.5, 	posxy + vec2(   sz, 0.0 ), sn, i*u32_upk(ub.v63, 24u, 0u));
		col = place(col, (sz/11.0)*((11.0-i))*0.5, 	posxy + vec2(  -sz, 0.0 ), sn, i*u32_upk(ub.v63, 24u, 0u));
		col = place(col, (sz/11.0)*((11.0-i))*0.5, 	posxy + vec2(  0.0,  sz ), sn, i*u32_upk(ub.v63, 24u, 0u));
		col = place(col, (sz/11.0)*((11.0-i))*0.5, 	posxy + vec2(  0.0, -sz ), sn, i*u32_upk(ub.v63, 24u, 0u)); }
	return col; }

vec4 conv( float r ) {
	ConvData nh = ring( r );
	return 	nh.value / nh.total; }

vec4 blendseed(vec4 col, uint seed, float str) {

	//		str 	 = str * reseed(seed + 17u, 0.8, 0.0) + str * 0.5;

	float 	amp 	 = 2.4 - (0.6 + str * 2.0);

	float	randr	 = reseed(seed + 0u, 1.0, amp);
	float	randg	 = reseed(seed + 1u, 1.0, amp);
	float	randb	 = reseed(seed + 3u, 1.0, amp);
	float	randa	 = reseed(seed + 5u, 1.0, amp);

	float 	blend	 = sqrt(reseed(seed + 7u, 0.3, 1.0) * reseed(seed + 11u, 0.6, 0.4)) + reseed(seed + 13u, 1.2, 0.0);

	float 	strsq	 = str * str;

			//col 	 = ( col 	- col	 * strsq 	 ) + conv( vec2(round(11.0*str)+1.0, 0.0), txdata ) * strsq;

			col[0]	 = ( col[0]	- col[0] * str * 0.5 ) + sqrt(randr * blend) * str;
			col[1]	 = ( col[1]	- col[1] * str * 0.5 ) + sqrt(randg * blend) * str;
			col[2]	 = ( col[2]	- col[2] * str * 0.5 ) + sqrt(randb * blend) * str;
			col[3]	 = ( col[3]	- col[3] * str * 0.5 ) + sqrt(randa * blend) * str;

			//col 	 = ( col 	- col	 * str 		 ) + conv( vec2(1.0, 0.0), txdata ) * str;

	return 	col; }

float tent_map(float x, float m, float a) {
	
	float result = 0.0;
	// if (x < a) { result = clamp(m*x/a, 0.0, 1.0); }  
	// if (x >= a) { result = clamp(m*(1.0-x)/(1.0-a), 0.0, 1.0); }
	result = (x < a) ? clamp(m*x/a, 0.0, 1.0) : clamp(m*(1.0-x)/(1.0-a), 0.0, 1.0);
	return result; 
	}	

void main() {

//	----    ----    ----    ----    ----    ----    ----    ----
//	Rule Initilisation
//	----    ----    ----    ----    ----    ----    ----    ----

//	Parameters
	const	float 	mnp 	= 1.0 / 65536.0;			//	Minimum value of a precise step for 16-bit channel
	const	float 	s  		= mnp *  80.0 * 128.0; // was 48
	const	float 	n  		= mnp *  80.0 *   2.0; // was 48

//	Output Values
	vec4 res_c = gdv( ivec2(0, 0), txdata );

//	Result Values
	vec4 res_v = res_c;

//	----    ----    ----    ----    ----    ----    ----    ----
//	Update Functions
//	----    ----    ----    ----    ----    ----    ----    ----
	uint[12] nb = uint[12] (
		ub.v0,  ub.v1,  ub.v2,  ub.v3,
		ub.v4,  ub.v5,  ub.v6,  ub.v7,
		ub.v8,  ub.v9,  ub.v10, ub.v11 );
	
	uint[24] ur = uint[24] (
		ub.v12, ub.v13, ub.v14, ub.v15, 
		ub.v16, ub.v17, ub.v18, ub.v19,	
		ub.v20, ub.v21, ub.v22, ub.v23,
		ub.v24, ub.v25, ub.v26, ub.v27,	
		ub.v28, ub.v29, ub.v30, ub.v31, 
		ub.v32, ub.v33, ub.v34, ub.v35  );	
// BLUR
	// res_c = conv( 1.0 );

// CGOL
	
	// res_c = gdv( ivec2(0, 0), txdata );
	// vec4[2] cgol_val = nbhd1( vec2(1.0,0.0), txdata );
	// vec4 sum = cgol_val[0];
	// vec4 tot = cgol_val[1];
	// vec4 res = sum / tot;

	// if(res_c[0] >= 0.5 ) { res_c[0] = 1.0; }
	// if(res_c[0] <  0.5 ) { res_c[0] = 0.0; }

	// if(res_c[1] >= 0.5 ) { res_c[1] = 1.0; }
	// if(res_c[1] <  0.5 ) { res_c[1] = 0.0; }

	// if(res_c[2] >= 0.5 ) { res_c[2] = 1.0; }
	// if(res_c[2] <  0.5 ) { res_c[2] = 0.0; }

	// if(res_c[3] >= 0.5 ) { res_c[3] = 1.0; }
	// if(res_c[3] <  0.5 ) { res_c[3] = 0.0; }

	// if(res[0] <= (1.0 / 8.0)) { res_c[0] = 0.0; }
	// if(res[0] >= (4.0 / 8.0)) { res_c[0] = 0.0; }
	// if(res[0] >= (3.0 / 8.0) && res[0] <= (3.0 / 8.0) ) { res_c[0] = 1.0; }

	// if(res[1] <= (1.0 / 8.0)) { res_c[1] = 0.0; }
	// if(res[1] >= (4.0 / 8.0)) { res_c[1] = 0.0; }
	// if(res[1] >= (3.0 / 8.0)   && res[1] <= (3.0 / 8.0) ) { res_c[1] = 1.0; }

	// if(res[2] <= (1.0 / 8.0)) { res_c[2] = 0.0; }
	// if(res[2] >= (4.0 / 8.0)) { res_c[2] = 0.0; }
	// if(res[2] >= (3.0 / 8.0)   && res[2] <= (3.0 / 8.0) ) { res_c[2] = 1.0; }

	// if(res[3] <= (1.0 / 8.0)) { res_c[3] = 0.0; }
	// if(res[3] >= (4.0 / 8.0)) { res_c[3] = 0.0; }
	// if(res[3] >= (3.0 / 8.0)   && res[3] <= (3.0 / 8.0) ) { res_c[3] = 1.0; }

// UNCOUPLED CML

	// res_c = gdv( ivec2(0, 0), txdata );

	// vec4[2] cgol_val = nbhd1( vec2(1.0,0.0), txdata );
	
	// vec4 sum = cgol_val[0];
	// vec4 tot = cgol_val[1];
	// vec4 res = sum / tot;

	// uint[3] ch  = uint[3] ( ub.v38, ub.v39, ub.v40 );
	
	// uint num_bits = 2u;
	
	// for(uint i = 0u; i < 4u; i++) {
		
	// 	uint m_i = u32_upk( ch[i], num_bits, 0*num_bits );
	// 	m_i = 2*m_i/(num_bits * num_bits -1);
		
	// 	uint a_i = u32_upk( ch[i], num_bits, 1*num_bits );
	// 	a_i = a_i/(num_bits * num_bits -1);
	// 	res_c[i] = tent_map(res[i], m_i, a_i);
	// }

// COUPLED CML


			
	// res_c = gdv( ivec2(0, 0), txdata );

	// vec4[2] cgol_val = nbhd1( vec2(1.0,0.0), txdata );
	
	// vec4 sum = cgol_val[0];
	// vec4 tot = cgol_val[1];
	// vec4 res = sum / tot;

	// uint[3] ch  = uint[3] ( ub.v38, ub.v39, ub.v40 );
	
	// uint num_bits = 4u;
	
	// for(uint i = 0u; i < 4u; i++) {
		
	// 	uint m_i = u32_upk( ch[i], num_bits, 0*num_bits );
	// 	m_i = num_bits*m_i/(num_bits * num_bits -1);
		
	// 	uint a_i = u32_upk( ch[i], num_bits, 1*num_bits );
	// 	a_i = a_i/(num_bits * num_bits -1);
		
	// 	// if (res[i] < a_i) {
	// 	// 	res_c[i] = clamp(m_i*res_c[i]/a_i, 0.0, 1.0); }  
	// 	// if (res[i] >= a_i) {
	// 	// 	res_c[i] = clamp(m_i*(1.0-res_c[i])/(1.0-a_i), 0.0, 1.0); }
	// 	res_c[i] = tent_map(res[i], m_i, a_i);
	// }
	// vec4 conv1 = conv(1.0);
	// float e = u32_upk( ub.v0, 1u, 0*num_bits )/10.0;
	// res_c = ((1-e)*res_v + e*(conv1*res_c));
/**/


// 	ConvData[MAX_RADIUS] nh_rings_m;
// 	for(uint i = 0u; i < MAX_RADIUS; i++) { nh_rings_m[i] = ring(i+1.0); }

// 	uint[12] nb = uint[12] (
// 		ub.v0,  ub.v1,  ub.v2,  ub.v3,
// 		ub.v4,  ub.v5,  ub.v6,  ub.v7,
// 		ub.v8,  ub.v9,  ub.v10, ub.v11 );

// 	uint[24] ur = uint[24] (
// 		ub.v12, ub.v13, ub.v14, ub.v15, 
// 		ub.v16, ub.v17, ub.v18, ub.v19,	
// 		ub.v20, ub.v21, ub.v22, ub.v23,
// 		ub.v24, ub.v25, ub.v26, ub.v27,	
// 		ub.v28, ub.v29, ub.v30, ub.v31, 
// 		ub.v32, ub.v33, ub.v34, ub.v35  );

// 	uint[ 3] ch2 = uint[ 3] ( 2286157824u, 295261525u, 1713547946u );
// 	uint[ 3] ch  = uint[ 3] ( ub.v38, ub.v39, ub.v40 );
// 	uint[ 3] ch3 = uint[ 3] ( ub.v41, ub.v42, ub.v43 );

// //	Update Sign
// 	uint[ 2] us = uint[ 2] ( ub.v36, ub.v37 );

// 	vec4[12] smnca_res = vec4[12](
// 		res_c,res_c,res_c,res_c,res_c,res_c,
// 		res_c,res_c,res_c,res_c,res_c,res_c
// 		);

// 	vec4 conv1 = conv(1.0);

// 	for(uint i = 0u; i < 12u; i++) 
		
// 		{

// 		uint  	cho = u32_upk( ch[i/8u], 2u, (i*4u+0u) & 31u );

// 				cho = (cho == 3u) ? 
// 					u32_upk( ch2[i/8u], 2u, (i*4u+0u) & 31u ) : cho;

// 		uint  	chi = u32_upk( ch[i/8u], 2u, (i*4u+2u) & 31u );

// 				chi = (chi == 3u) ? 
// 					u32_upk( ch2[i/8u], 2u, (i*4u+2u) & 31u ) : chi;
					
// 		uint  	chm = u32_upk( ch3[i/8u], 2u, (i*4u+2u) & 31u );

// 				chm = (chm == 3u) ? 
// 					u32_upk( ch[i/8u], 2u, (i*4u+2u) & 31u ) : chm;

// 		//float nhv = bitring( nh_rings_c, nb[i/2u], (i & 1u) * 16u )[cho];
// 		vec4 nhv = bitmake( nh_rings_m, nb[i/2u], (i & 1u) * 16u );

// 		if( nhv[cho] >= utp( ur[i], 8u, 0u) && nhv[cho] <= utp( ur[i], 8u, 1u)) {
// 			smnca_res[i/4u][chi] += bsn(us[i/16u], ((i*2u+0u) & 31u)) * s * res_c[chm]; }

// 		if( nhv[cho] >= utp( ur[i], 8u, 2u) && nhv[cho] <= utp( ur[i], 8u, 3u)) {
// 			smnca_res[i/4u][chi] += bsn(us[i/16u], ((i*2u+1u) & 31u)) * s * res_c[chm]; } 
			
// 		}

// 	uvec4 dev_idx = uvec4(0u,0u,0u,0u);
// //	vec4 dev = vec4(2.0,2.0,2.0,2.0);
// 	vec4 dev = vec4(0.0,0.0,0.0,0.0);
// 	for(uint i = 0u; i < 6u; i++) {
// 		vec4 smnca_res_temp = abs(res_c - smnca_res[i]);
// 		if(smnca_res_temp[0] > dev[0]) { dev_idx[0] = i; dev[0] = smnca_res_temp[0]; }
// 		if(smnca_res_temp[1] > dev[1]) { dev_idx[1] = i; dev[1] = smnca_res_temp[1]; }
// 		if(smnca_res_temp[2] > dev[2]) { dev_idx[2] = i; dev[2] = smnca_res_temp[2]; }
// 		if(smnca_res_temp[3] > dev[3]) { dev_idx[3] = i; dev[3] = smnca_res_temp[3]; } }

// 	res_v[0] = smnca_res[dev_idx[0]][0];
// 	res_v[1] = smnca_res[dev_idx[1]][1];
// 	res_v[2] = smnca_res[dev_idx[2]][2];
// 	res_v[3] = smnca_res[dev_idx[3]][3];
// 	res_c = ((res_v + (conv1 * (s*2.0))) / (1.0 + (s*2.13333)))- 0.015 * s;


// Original Update

// 	ConvData[MAX_RADIUS] nh_rings_m;
// 	for(uint i = 0u; i < MAX_RADIUS; i++) { nh_rings_m[i] = ring(i+1.0); }

// 	uint[12] nb = uint[12] (
// 		ub.v0,  ub.v1,  ub.v2,  ub.v3,
// 		ub.v4,  ub.v5,  ub.v6,  ub.v7,
// 		ub.v8,  ub.v9,  ub.v10, ub.v11 );

// 	uint[24] ur = uint[24] (
// 		ub.v12, ub.v13, ub.v14, ub.v15, 
// 		ub.v16, ub.v17, ub.v18, ub.v19,	
// 		ub.v20, ub.v21, ub.v22, ub.v23,
// 		ub.v24, ub.v25, ub.v26, ub.v27,	
// 		ub.v28, ub.v29, ub.v30, ub.v31, 
// 		ub.v32, ub.v33, ub.v34, ub.v35  );

// 	uint[ 3] ch2 = uint[ 3] ( 2286157824u, 295261525u, 1713547946u );
// 	uint[ 3] ch  = uint[ 3] ( ub.v38, ub.v39, ub.v40 );
// 	uint[ 3] ch3 = uint[ 3] ( ub.v41, ub.v42, ub.v43 );

// //	Update Sign
// 	uint[ 2] us = uint[ 2] ( ub.v36, ub.v37 );

// 	vec4[12] smnca_res = vec4[12](
// 		res_c,res_c,res_c,res_c,res_c,res_c,
// 		res_c,res_c,res_c,res_c,res_c,res_c
// 		);

// 	vec4 conv1 = conv(1.0);

// 	for(uint i = 0u; i < 24u; i++) 
		
// 		{

// 		uint  	cho = u32_upk( ch[i/8u], 2u, (i*4u+0u) & 31u );

// 				cho = (cho == 3u) ? 
// 					u32_upk( ch2[i/8u], 2u, (i*4u+0u) & 31u ) : cho;

// 		uint  	chi = u32_upk( ch[i/8u], 2u, (i*4u+2u) & 31u );

// 				chi = (chi == 3u) ? 
// 					u32_upk( ch2[i/8u], 2u, (i*4u+2u) & 31u ) : chi;
					
// 		uint  	chm = u32_upk( ch3[i/8u], 2u, (i*4u+2u) & 31u );

// 				chm = (chm == 3u) ? 
// 					u32_upk( ch[i/8u], 2u, (i*4u+2u) & 31u ) : chm;

// 		//float nhv = bitring( nh_rings_c, nb[i/2u], (i & 1u) * 16u )[cho];
// 		vec4 nhv = bitmake( nh_rings_m, nb[i/2u], (i & 1u) * 16u );

// 		if( nhv[cho] >= utp( ur[i], 8u, 0u) && nhv[cho] <= utp( ur[i], 8u, 1u)) {
// 			smnca_res[i/4u][chi] += bsn(us[i/16u], ((i*2u+0u) & 31u)) * s * res_c[chm]; }

// 		if( nhv[cho] >= utp( ur[i], 8u, 2u) && nhv[cho] <= utp( ur[i], 8u, 3u)) {
// 			smnca_res[i/4u][chi] += bsn(us[i/16u], ((i*2u+1u) & 31u)) * s * res_c[chm]; } 
			
// 		}

// 	uvec4 dev_idx = uvec4(0u,0u,0u,0u);
// //	vec4 dev = vec4(2.0,2.0,2.0,2.0);
// 	vec4 dev = vec4(0.0,0.0,0.0,0.0);
// 	for(uint i = 0u; i < 6u; i++) {
// 		vec4 smnca_res_temp = abs(res_c - smnca_res[i]);
// 		if(smnca_res_temp[0] > dev[0]) { dev_idx[0] = i; dev[0] = smnca_res_temp[0]; }
// 		if(smnca_res_temp[1] > dev[1]) { dev_idx[1] = i; dev[1] = smnca_res_temp[1]; }
// 		if(smnca_res_temp[2] > dev[2]) { dev_idx[2] = i; dev[2] = smnca_res_temp[2]; }
// 		if(smnca_res_temp[3] > dev[3]) { dev_idx[3] = i; dev[3] = smnca_res_temp[3]; } }

// 	res_v[0] = smnca_res[dev_idx[0]][0];
// 	res_v[1] = smnca_res[dev_idx[1]][1];
// 	res_v[2] = smnca_res[dev_idx[2]][2];
// 	res_v[3] = smnca_res[dev_idx[3]][3];
// 	res_c = ((res_v + (conv1 * (s*2.0))) / (1.0 + (s*2.13333)))- 0.015 * s;


//	----    ----    ----    ----    ----    ----    ----    ----
//	Shader Output
//	----    ----    ----    ----    ----    ----    ----    ----

	if( u32_upk(ub.v63, 24u, 0u) <= 0u
	||	u32_upk(ub.v60, 6u, 26u) == 1u ) {
		res_c[0] = reseed( u32_upk(ub.v63, 8u, 24u) + 0u, 1.0, 0.4 ); 
		res_c[1] = reseed( u32_upk(ub.v63, 8u, 24u) + 1u, 1.0, 0.4 ); 
		res_c[2] = reseed( u32_upk(ub.v63, 8u, 24u) + 2u, 1.0, 0.4 ); 
		res_c[3] = reseed( u32_upk(ub.v63, 8u, 24u) + 3u, 1.0, 0.4 ); }

	if( u32_upk(ub.v60, 6u, 26u) == 2u ) {
		res_c[0] = 0.0; 
		res_c[1] = 0.0; 
		res_c[2] = 0.0; 
		res_c[3] = 1.0; }

	if( u32_upk(ub.v60, 6u, 26u) == 3u ) {
		res_c = symsd(res_c, 128.0); }

	if( u32_upk(ub.v60, 6u, 26u) == 4u ) {
		res_c = blendseed(res_c, u32_upk(ub.v63, 8u, 24u), vmap() * cmap() * 1.4 ); }

	if(u32_upk(ub.v60, 2u, 24u) != 0u) {
		res_c = mouse(res_c, 38.0);	}

//	Force alpha to 1.0
	res_c[3] 	= 1.0;

	out_col 	= res_c;

}


