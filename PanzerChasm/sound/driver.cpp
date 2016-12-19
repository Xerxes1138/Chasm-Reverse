#include <limits>

#include <SDL.h>

#include "../assert.hpp"
#include "../log.hpp"

#include "driver.hpp"

namespace PanzerChasm
{

namespace Sound
{

static const SDL_AudioDeviceID g_first_valid_device_id= 2u;
static const unsigned int g_left_and_right= 2u;

static int NearestPowerOfTwoFloor( int x )
{
	int r= 1 << 30;
	while( r > x ) r>>= 1;
	return r;
}

Driver::Driver()
{
	SDL_InitSubSystem( SDL_INIT_AUDIO );

	SDL_AudioSpec requested_format;
	SDL_AudioSpec obtained_format;

	requested_format.channels= g_left_and_right;
	requested_format.freq= 22050;
	requested_format.format= AUDIO_S16;
	requested_format.callback= AudioCallback;
	requested_format.userdata= this;

	// ~ 1 callback call per two frames (60fps)
	requested_format.samples= NearestPowerOfTwoFloor( requested_format.freq / 30 );

	int device_count= SDL_GetNumAudioDevices(0);
	// Can't get explicit devices list. Trying to use first device.
	if( device_count == -1 )
		device_count= 1;

	for( int i= 0; i < device_count; i++ )
	{
		const char* const device_name= SDL_GetAudioDeviceName( i, 0 );

		const SDL_AudioDeviceID device_id=
			SDL_OpenAudioDevice( device_name, 0, &requested_format, &obtained_format, 0 );

		if( device_id >= g_first_valid_device_id &&
			obtained_format.channels == requested_format.channels &&
			obtained_format.format   == requested_format.format )
		{
			device_id_= device_id;
			Log::Info( "Open audio device: ", device_name );
			break;
		}
	}

	if( device_id_ < g_first_valid_device_id )
	{
		Log::FatalError( "Can not open any audio device" );
		return;
	}

	frequency_= obtained_format.freq;

	mix_buffer_.resize( requested_format.samples * g_left_and_right );
}

Driver::~Driver()
{
	if( device_id_ >= g_first_valid_device_id )
		SDL_CloseAudioDevice( device_id_ );


	SDL_QuitSubSystem( SDL_INIT_AUDIO );
}

void Driver::LockChannels()
{
	if( device_id_ >= g_first_valid_device_id )
		SDL_LockAudioDevice( device_id_ );
}

void Driver::UnlockChannels()
{
	if( device_id_ >= g_first_valid_device_id )
		SDL_UnlockAudioDevice( device_id_ );
}

Channels& Driver::GetChannels()
{
	return channels_;
}

void SDLCALL Driver::AudioCallback( void* userdata, Uint8* stream, int len_bytes )
{
	Driver* const self= reinterpret_cast<Driver*>( userdata );

	self->FillAudioBuffer(
		reinterpret_cast<SampleType*>( stream ),
		static_cast<unsigned int>( len_bytes ) / ( sizeof(SampleType) * g_left_and_right ) );
}

void Driver::FillAudioBuffer( SampleType* const buffer, const unsigned int sample_count )
{
	// Zero mix buffer.
	for( unsigned int i= 0u; i < sample_count * 2u; i++ )
		mix_buffer_[i]= 0;

	for( const Channel& channel : channels_ )
	{
		PC_UNUSED( channel );
		// TODO  - mix to buffer here
	}

	// Copy mix buffer to result buffer.
	for( unsigned int i= 0u; i < sample_count * 2u; i++ )
	{
		int s= mix_buffer_[i];
		if( s > +32767 ) s= +32767;
		if( s < -32767 ) s= -32767;
		buffer[i]= s;
	}
}

} // namespace Sound

} // namespace PanzerChasm