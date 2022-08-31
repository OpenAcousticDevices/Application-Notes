import pyaudio
import wave

audio_format = pyaudio.paInt16
number_of_channels = 1
sample_rate = 192000
chunk_size = 4096 
duration = 60

filename = 'test.wav'

# Create pyaudio instance and search for the AudioMoth

device_index = None

audio = pyaudio.PyAudio() 

for i in range(audio.get_device_count()):
    if 'AudioMoth' in audio.get_device_info_by_index(i).get('name'):
        device_index = i
        break

if device_index == None:
    print('No AudioMoth found')
    exit()
    
# Create pyaudio stream

stream = audio.open(format=audio_format, rate=sample_rate, channels=number_of_channels, input_device_index=device_index, input=True, frames_per_buffer=chunk_size)

# Append audio chunks to array until enough samples have been acquired

print('Start recording')

data = []

total_samples = sample_rate * duration

while total_samples > 0:
    samples = min(total_samples, chunk_size)
    data.append(stream.read(samples))
    total_samples -= samples

print('Finished recording')

# Stop the stream, close it, and terminate the pyaudio instance

stream.stop_stream()
stream.close()
audio.terminate()

# Save the audio data as a WAV file

wavefile = wave.open(filename, 'wb')
wavefile.setnchannels(number_of_channels)
wavefile.setsampwidth(audio.get_sample_size(audio_format))
wavefile.setframerate(sample_rate)
wavefile.writeframes(b''.join(data))
wavefile.close()