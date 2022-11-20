# minitask 2
# change the last du to DU
import re

pattern = re.compile(r"\bdu(?!.*?\bdu\b)\b")
text = ["du du du", "du po ledu", "dopředu du", "i dozadu du", "dudu dupl", "Rammstein du hast"]
for row in text:
    print(re.sub(pattern, "DU", row))
