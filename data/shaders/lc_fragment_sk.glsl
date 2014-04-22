#version 420
#extension GL_EXT_gpu_shader4 : enable

in vec3 outColor;
in vec3 outUV;
in vec3 origVertex;// L

in vec3 varyingNormal;
in vec3 varyingLightDirection;
in vec4 varyingLightVertexPosition;//position respect the camera view

out vec4 outputF;

uniform int mode;
uniform sampler2D tex0;
uniform sampler2DArray tex_3D;
//uniform sampler3D tex_3D;


uniform sampler2D shadowMap;
uniform int shadowState;
 
uniform mat4 light_mvpMatrix;
uniform mat4 light_biasMatrix;	//transfrom to [0,1] tex coordinates

uniform float waterMove;

const float ambientColor=0.1;
const float diffuseColor=1.0;
const float specularColor=1.0;

vec2 poissonDisk4[4] = vec2[](
  vec2( -0.94201624, -0.39906216 ),
  vec2( 0.94558609, -0.76890725 ),
  vec2( -0.094184101, -0.92938870 ),
  vec2( 0.34495938, 0.29387760 )
);

vec2 poissonDisk16[16] = vec2[]( 
   vec2( -0.94201624, -0.39906216 ), 
   vec2( 0.94558609, -0.76890725 ), 
   vec2( -0.094184101, -0.92938870 ), 
   vec2( 0.34495938, 0.29387760 ), 
   vec2( -0.91588581, 0.45771432 ), 
   vec2( -0.81544232, -0.87912464 ), 
   vec2( -0.38277543, 0.27676845 ), 
   vec2( 0.97484398, 0.75648379 ), 
   vec2( 0.44323325, -0.97511554 ), 
   vec2( 0.53742981, -0.47373420 ), 
   vec2( -0.26496911, -0.41893023 ), 
   vec2( 0.79197514, 0.19090188 ), 
   vec2( -0.24188840, 0.99706507 ), 
   vec2( -0.81409955, 0.91437590 ), 
   vec2( 0.19984126, 0.78641367 ), 
   vec2( 0.14383161, -0.14100790 ) 
);

// Returns a random number based on a vec3 and an int.
float random(vec3 seed, int i){
	vec4 seed4 = vec4(seed,i);
	float dot_product = dot(seed4, vec4(12.9898,78.233,45.164,94.673));
	return fract(sin(dot_product) * 43758.5453);
}

float shadowCoef(){
	vec4 shadow_coord2=light_mvpMatrix*vec4(origVertex,1.0);
	vec3 ProjCoords = shadow_coord2.xyz / shadow_coord2.w;
    vec2 UVCoords;
    UVCoords.x = 0.5 * ProjCoords.x + 0.5;
    UVCoords.y = 0.5 * ProjCoords.y + 0.5;
    float z = 0.5 * ProjCoords.z + 0.5;
	
	/// D
	bool HDShadow=false;
	float visibility=1.0f;
	if(HDShadow==true){
		// HDShadow
		for (int i=0;i<8;i++){
			int index = int(16.0*random(origVertex.xyz, i))%16;
			if ( texture2D( shadowMap, UVCoords + poissonDisk16[index]/3500.0 ).z  <  z ){
				visibility-=0.1;
			}
		}
	}else{
		// Low Shadow
		for (int i=0;i<4;i++){
			int index = int(4.0*random(origVertex.xyz, i))%4;
			if ( texture2D( shadowMap, UVCoords + poissonDisk4[index]/3500.0 ).z  <  z ){
				visibility-=0.2;
			}
		}

	}
	return visibility;
}

float facade(vec2 coordFac,vec2 maxFac,int windNumber){
	float intVal=1.0f;
	
	// SIDES
	if(coordFac.x<2.5||coordFac.y<2.5){
		//return 0;
		intVal= smoothstep(1.5,2.5,min(coordFac.x,coordFac.y));
	}
	if(coordFac.x>maxFac.x-2.5||coordFac.y>maxFac.y-2.5){
		float coord=min(maxFac.x-coordFac.x,maxFac.y-coordFac.y);
		intVal= min(intVal,smoothstep(1.5,2.5,coord));
	}
	if(intVal==0)
		return 0;
	// WINDOWS
	int numSt=int(ceil(maxFac.y*0.26f));//3.0m
	int numWin=int(ceil(maxFac.x*0.26f));
	//if(int(numWin*coordFac.x/maxFac.x)%2==0)
	//	return 1.0;
	
	int currS=int((numSt*coordFac.y)/maxFac.y);
	if(currS==0||currS==numSt-1)//side windows
		return intVal;
	//float currS=(numSt*coordFac.y)/maxFac.y;
	//if(currS<2.0)//first floor
	//	return max(intVal-(2.0-currS),0);
	int currWind=int((numWin*coordFac.x)/maxFac.x);
	if(currWind==0||currWind==numWin-1)//side windows
		return intVal;
	
	return intVal*texture( tex_3D, vec3(vec2(numWin,numSt)*coordFac.xy/maxFac.xy,7.0) )[windNumber];//6 window
	//return intVal;

}//

void main(){



	outputF =vec4(outColor,1.0);

	// SHADOW: From light
	if(shadowState==2){// no tex/color/shadows
		return;
	}

	// COLOR
	/*if(mode==1){
		outputF =vec4(outColor,1.0);
	}*/

	// TEXTURE
	if((mode&0xFF)==2||(mode&0xFF)==4||(mode&0xFF)==6){// tex / water / model texture
		outputF = texture( tex0, outUV.rg );
	}
	
	//////////////
	// TERRAIN
	if((mode&0xFF)==3){
		/*float height=(origVertex.z)/100.0;
		outputF = texture( tex_3D, vec3(outUV.rg,height) );*/
		//outputF = texture( tex_3D, vec3(outUV.rg,3.0) );
		//return;

		vec4 terrainColor=vec4(0,0,0,1.0);
		float factor;
		//float height=(origVertex.z)/255.0;//0-1
		float height=(origVertex.z)/255.0;//0-1
		height=min(height,1.0);
		height=max(height,0);
		/*terrainColor+=height*texture( tex_3D, vec3(outUV.rg,1.0));
		terrainColor+=(1-height)*texture( tex_3D, vec3(outUV.rg,2.0));//texture( tex_3D, vec3(outUV.rg,2.0));
		outputF =terrainColor;
		return;*/
		if(height<=0.33){
			//terrainColor=vec4(1.0,1.0,1.0,1.0);
			factor=height/0.33;//0-1
			//terrainColor+=(1-factor)*texture( tex_3D, vec3(outUV.rg,0.0));
			//terrainColor+=factor*texture( tex_3D, vec3(outUV.rg,1.0));
			terrainColor+=(1-factor)*texture( tex_3D, vec3(outUV.rg,0.0));
			terrainColor+=factor*texture( tex_3D, vec3(outUV.rg+vec2(0.1,0.1),1.0));
		}else{
			if(height<=0.66){
				//terrainColor=vec4(0,1.0,1.0,1.0);
				factor=(height-0.33)/0.33f;//0-1
				//terrainColor+=(1-factor)*texture( tex_3D, vec3(outUV.rg,1.0));
				//terrainColor+=factor*texture( tex_3D, vec3(outUV.rg,2.0));
				terrainColor+=(1-factor)*texture( tex_3D, vec3(outUV.rg+vec2(0.1,0.1),1.0));
				terrainColor+=factor*texture( tex_3D, vec3(outUV.rg+vec2(0.45,0.45),2.0));
			}else{
				//terrainColor=vec4(0,0,1.0,1.0);
				factor=(height-0.66)/0.33f;//0-1
				//terrainColor+=(1-factor)*texture( tex_3D, vec3(outUV.rg,2.0));
				//terrainColor+=factor*texture( tex_3D, vec3(outUV.rg,3.0));
				terrainColor+=(1-factor)*texture( tex_3D, vec3(outUV.rg+vec2(0.45,0.45),2.0));
				terrainColor+=factor*texture( tex_3D, vec3(outUV.rg+vec2(0.3,0.3),3.0));

			}
		}
		outputF =terrainColor;
	}

	//////////////
	// FACADE
	if((mode&0xFF)==0x08){
		outputF =vec4(1.0,1.0,1.0,1.0);
		vec2 coordFac=outColor.xy;
		vec2 maxFac=outUV.xy;
		float dist=2.0;
		if(coordFac.x<2.0||coordFac.y<2.0){
			dist=min(coordFac.x,coordFac.y);
		}
		if(coordFac.x>maxFac.x-2.0||coordFac.y>maxFac.y-2.0){
			dist=min(dist,min(maxFac.x-coordFac.x,maxFac.y-coordFac.y));
		}
		float t=smoothstep(1.5,2.0,dist);
		outputF =mix(texture( tex0, coordFac.xy*0.1 ),vec4(1,1,1,1),t);
		
		//float deriv=fwidth(coordFac);
		//float amt = smoothstep(1-deriv,.1+deriv,coordFac.x);
		//outputF =mix(vec4(0.0,0.0,0.0,1.0),vec4(1.0,1.0,1.0,1.0),amt);
		return;
	}

	//////////////
	// HATCH WATER
	if((mode&0xFF)==11){
		vec2 coordFac=outUV.xy;
		//vec2 coord0=coordFac+vec2(waterMove);
		vec2 coord0=(coordFac+vec2(waterMove*3))*0.33;
		vec2 coord1=(coordFac+vec2(-waterMove*4))*0.25;
		vec3 normalC=texture( tex_3D, vec3(coord0,6.0) ).xyz;//6 water normal
		vec3 normalC2=texture( tex_3D, vec3(coord1,6.0) ).xyz;//6 water normal

		normalC = normalC*2.0-vec3(1.0);
		normalC2 = normalC2*2.0-vec3(1.0);
		normalC = normalize((normalC+normalC2)*0.5);
		/*vec2 coord0=(coordFac+vec2(-waterMove,waterMove))*1.02;
		vec2 coord1=(coordFac+vec2(waterMove*3))*0.33;
		vec2 coord2=(coordFac+vec2(-waterMove*4))*0.25;
		vec3 normalC=texture( tex_3D, vec3(coord0,6.0) ).xyz;//6 water normal
		vec3 normalC2=texture( tex_3D, vec3(coord1,6.0) ).xyz;//6 water normal
		vec3 normalC3=texture( tex_3D, vec3(coord2,6.0) ).xyz;//6 water normal

		normalC = normalC*2.0-vec3(1.0);
		normalC2 = normalC2*2.0-vec3(1.0);
		normalC3 = normalC3*2.0-vec3(1.0);
		normalC = normalize((normalC+normalC2+normalC3)*0.33);*/
		/*int numWav=2;
		vec3 normalC=vec3(0,0,0);
		for(int i=3;i<3+numWav;i++){
			float sig=i%2==0?-1.0:1.0;
			vec2 coord=(coordFac+vec2(sig*waterMove*i))/i;
			normalC+=texture( tex_3D, vec3(coord,6.0) ).xyz;//6 water normal
		}
		normalC=normalize((2.0*normalC-vec3(1.0))/numWav);*/
	
		float intensity=1.0f;

		intensity=1-(0.95*max(0.0, dot(normalize(varyingLightDirection), normalize(normalC)))+0.05);
		//intensity=clamp(intensity,0,1);
		//intensity=(intensity)-0.1;
		int darknessInt=int(99.999*(1.0-intensity));
		int texInd=darknessInt/25;
		int interpTex=darknessInt%25;
		outputF=mix(
			texture( tex_3D, vec3(coordFac.xy*0.1,texInd) ),
			texture( tex_3D, vec3(coordFac.xy*0.1,texInd+1) ),
			interpTex/25.0);
		return;
	}
	//////////////
	// FACADE ARRAY TEXTURE
	if((mode&0xFF)==9||(mode&0xFF)==10){
		float intensity=1.0f;

		vec2 coordFac=outColor.xy;
		vec2 maxFac=outUV.xy;
		if((mode&0xFF)==10){
			coordFac=outUV.xy;
			intensity=outColor.x;
		}
		
		if((mode&0x0200)==0x0200){
			intensity=1-(0.95*max(0.0, dot(normalize(varyingLightDirection), normalize(varyingNormal)))+0.05);
			intensity=clamp(intensity,0,1);
		}
		if((mode&0xFF)==9){
			int windNumber=int(outColor.z);
			intensity*=facade(coordFac,maxFac,windNumber);
			/*//float dist=2.0;
			if(coordFac.x<2.0||coordFac.y<2.0){
				//dist=min(coordFac.x,coordFac.y);
				intensity=0;
			}
			if(coordFac.x>maxFac.x-2.0||coordFac.y>maxFac.y-2.0){
				//dist=min(dist,min(maxFac.x-coordFac.x,maxFac.y-coordFac.y));
				intensity=0;
			}*/
		}
		/*float t=smoothstep(1.5,2.0,dist);
		intensity=clamp(intensity-t,0,1);*/


		int darknessInt=int(99.999*(1.0-intensity));
		int texInd=darknessInt/25;
		int interpTex=darknessInt%25;
		outputF=mix(
			texture( tex_3D, vec3(coordFac.xy*0.1,texInd) ),
			texture( tex_3D, vec3(coordFac.xy*0.1,texInd+1) ),
			interpTex/25.0);
		
		if(texInd<2){
			float perl=texture( tex_3D, vec3(coordFac.xy/maxFac.xy,5.0) ).x;//5.0 perlin noise
			outputF=mix(
				outputF,
				vec4(1,1,1,1),
				perl*0.3*(4-texInd));
		}

		/*if(intensity<=0.2){
			outputF =vec4(1.0,1.0,1.0,1.0);
		}else{
			if(intensity<=0.3){
				outputF=mix(vec4(1.0,1.0,1.0,1.0),texture( tex_3D, vec3(coordFac.xy*0.1,0.0) ),(intensity-0.2)*10.0);
			}else{
				if(intensity<=0.4){
					outputF=texture( tex_3D, vec3(coordFac.xy*0.1,0.0) );
				}else{
					if(intensity<=0.6){
						outputF=mix(texture(tex_3D, vec3(coordFac.xy*0.1,0.0)),texture( tex_3D, vec3(coordFac.xy*0.1,1.0) ),(intensity-0.4)*10.0);
					}else{
						if(intensity<=0.7){
							outputF=texture( tex_3D, vec3(coordFac.xy*0.1,1.0) );
						}else{
							if(intensity<=0.8){
								outputF=mix(texture(tex_3D, vec3(coordFac.xy*0.1,1.0)),texture( tex_3D, vec3(coordFac.xy*0.1,2.0) ),(intensity-0.7)*10.0);
							}else{
								outputF=texture( tex_3D, vec3(coordFac.xy*0.1,2.0));

							}

						}

					}

				}
			}
		}*/
		/*outputF =vec4(1.0,1.0,1.0,1.0);
		vec2 coordFac=outColor.xy;
		vec2 maxFac=outUV.xy;
		float dist=2.0;
		if(coordFac.x<2.0||coordFac.y<2.0){
			dist=min(coordFac.x,coordFac.y);
		}
		if(coordFac.x>maxFac.x-2.0||coordFac.y>maxFac.y-2.0){
			dist=min(dist,min(maxFac.x-coordFac.x,maxFac.y-coordFac.y));
		}
		float t=smoothstep(1.5,2.0,dist);
		outputF =mix(texture( tex_3D, vec3(coordFac.xy*0.1,0.0) ),vec4(1,1,1,1),t);*/
		
		return;
	}


	// LIGHTING
	vec4 ambientIllumination=vec4(0.05,0.05,0.05,0.0);
	vec4 diffuseIllumination=vec4(0.95,0.95,0.95,1.0);
	if((mode&0x0200)==0x0200){
		vec3 normal = normalize(varyingNormal);
		//vec3 lightDirection = normalize(vec3(0.5,0.5,0.5));//normalize(varyingLightDirection);
		vec3 lightDirection = normalize(varyingLightDirection);
		ambientIllumination = ambientColor*vec4(1.0,1.0,1.0,1.0);
		diffuseIllumination = (diffuseColor*vec4(1.0,1.0,1.0,1.0)) * max(0.0, dot(lightDirection, normal));
		//outputF=(ambientIllumination+diffuseIllumination)*outputF;
	}

	// SHADOW Disable
	if(shadowState==0){// 0 SHADOW Disable
		outputF=(ambientIllumination+diffuseIllumination)*outputF;
		return;
	}

	// SHADOW Render normal with shadows
	if(shadowState==1){// 1 SHADOW Render Normal
		float shadow_coef=0.95;
		shadow_coef= shadowCoef();
		outputF=(ambientIllumination+(shadow_coef+0.05)*diffuseIllumination)*outputF;
		return;
	}

}//

