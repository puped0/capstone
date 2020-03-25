import pygame

def tts_func(line):
	from google.cloud import texttospeech
	client = texttospeech.TextToSpeechClient()
	input = texttospeech.types.SynthesisInput(text=line)
	voice = texttospeech.types.VoiceSelectionParams(
    	language_code='ko-KR',
		name='ko-KR-Standard-B',
    	ssml_gender=texttospeech.enums.SsmlVoiceGender.NEUTRAL)

	audio_config = texttospeech.types.AudioConfig(
   		audio_encoding=texttospeech.enums.AudioEncoding.MP3)

	response = client.synthesize_speech(input, voice, audio_config)

	with open('output.mp3', 'wb') as out:
    	# Write the response to the output file.
   		out.write(response.audio_content)
	
	music_file = 'output.mp3'

	freq = 24000    # sampling rate, 44100(CD), 16000(Naver TTS), 24000(google TTS
	bitsize = -16   # signed 16 bit. support 8,-8,16,-16
	channels = 1    # 1 is mono, 2 is stereo
	buffer = 2048   # number of samples (experiment to get right sound)

	# default : pygame.mixer.init(frequency=22050, size=-16, channels=2, buffer=40
	pygame.mixer.init(freq, bitsize, channels, buffer)
	pygame.mixer.music.load(music_file)
	pygame.mixer.music.play()

	clock = pygame.time.Clock()
	while pygame.mixer.music.get_busy():
	    clock.tick(30)
	pygame.mixer.quit()
	return 0
