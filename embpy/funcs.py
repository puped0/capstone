def multiply():
	a = 12
	b = 20
	print("Will compute", a, "times", b)
	c = 0
	for i in range(0, a):
		c = c + b;
	return c

def xmlparse():
	import xml.etree.ElementTree as et
	tree = et.parse('country_data.xml')

	root = tree.getroot()

	c = root.findall('country')
	print(c)
	return 0
<<<<<<< HEAD
=======

def tts_ex(index, voice, line):
    print(index, type(index))
    print(voice, type(voice))
    print(line, type(line))

def print_line(line):
    print("module : ", line)

>>>>>>> 74e8d524d90b7821b1ea78896a04a61b7ba97b6e
