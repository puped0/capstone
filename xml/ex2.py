

import xml.etree.ElementTree as elemTree

tree = elemTree.parse('users.xml')

user = tree.find('./user[1]')

print(user.tag)
print(user.attrib)
print(user.get('grade'))

username = user.find('name')
print(username.text)

