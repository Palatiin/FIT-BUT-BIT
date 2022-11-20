# minitask 1
# modify the regular expression below so that
# it captures any string inside each <...>
# expected output ['/note', 'b', '/b', 'i', '/i']
import re
text = '</note> and <b>foo</b> and <i>so on</i> and 1 < 2'
print(re.findall(r'<(.+?)>', text))
