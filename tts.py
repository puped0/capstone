
import pygame
import time

def tts_func(index, voice, line):
    filename = 'output'+str(index)+'.wav';
    
    from google.cloud import texttospeech
    
    client = texttospeech.TextToSpeechClient()
    input = texttospeech.types.SynthesisInput(text=line)
    voice = texttospeech.types.VoiceSelectionParams(
    language_code='ko-KR',
        name=voice,
        ssml_gender=texttospeech.enums.SsmlVoiceGender.NEUTRAL)

    audio_config = texttospeech.types.AudioConfig(
            audio_encoding=texttospeech.enums.AudioEncoding.LINEAR16)	
    response = client.synthesize_speech(input, voice, audio_config)

    with open(filename, 'wb') as out:
    # Write the response to the output file.
        out.write(response.audio_content)
    return 0

def line_play(index):
    music_file = []

    freq = 24000
    bitsize = -16
    channels = 1
    buffer = 2048

    pygame.mixer.init(freq, bitsize, channels, buffer)
    
    for i in range(index):
        print('output'+str(i)+'.wav playing...')
        music_file.append(pygame.mixer.Sound('output'+str(i)+'.wav'))

    for i in range(index):
        print(music_file[i])
        music_file[i].play()
   
    while pygame.mixer.get_busy():
       time.sleep(1)
   
    pygame.mixer.quit()


