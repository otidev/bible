import os
import sys
import re
import xml.etree.ElementTree as ET

print(f"Input file: {sys.argv[1]}")
inputText = open(sys.argv[1], "r").read()

placeAndVerse = re.split(r'[\t\n]', inputText)

# print(placeAndVerse)
placeAndVerse.pop(len(placeAndVerse) - 1)

print(len(sys.argv))

if (len(sys.argv) == 4):
    language = sys.argv[3]
else:
    language = "en"

xmlText = ET.Element("bible", lang=language)
index = 0
bookName = ""
bookNumber = 1
chapterNumber = 0
verseNumber = 1
for i, item in enumerate(placeAndVerse):
    if (i < len(placeAndVerse) - 1):
        if (index == 0):
            if (bookName != placeAndVerse[i]):
                xmlBook = ET.SubElement(xmlText, "b")
                bookName = placeAndVerse[i]
                bookNumber = bookNumber + 1
                chapterNumber = 0
                verseNumber = 0
        if (index == 1):
            if (chapterNumber != int(placeAndVerse[i])):
                chapterNumber = chapterNumber + 1
                xmlChap = ET.SubElement(xmlBook, "c", n=f"{chapterNumber}")
        if (index == 2):
            verseNumber = int(placeAndVerse[i])
            xmlVerse = ET.SubElement(xmlChap, "v", n=f"{item}").text = placeAndVerse[i + 1]

        if (index == 3):
            index = -1
        index = index + 1
ET.ElementTree(xmlText).write(sys.argv[2])
