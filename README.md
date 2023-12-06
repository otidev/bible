# Bible
A (kinda) fast Bible that anyone can use.

## Usage

``Arrows left/right`` - Change chapter

``Ctrl + F`` - Change Bible book and/or chapter (kinda buggy)

``Ctrl + L`` - Toggle verse-per-line reading

``Scroll wheel/Up arrow/Down arrow`` - Scroll (hold ``Ctrl`` while you scroll to enlargen)

``Ctrl + D`` - Toggle dark mode

``=/-`` - Increase/Decrease wrap width

To change Bible versions, type `./bible -v <version>`.

Currently, you can select from these versions:

* BBE
* KJV
* AKJV (American KJV)
* UKJV (Updated KJV)
* WEB

***

In the future, any file with the format:

```xml
<bible lang="<language in Books.json>">
    <b> <!-- book -->
        <c n="<chapterNumber>"> <!-- chapter -->
            <v n="<verseNumber>"> <!-- verse -->
            </v>
        </c>
    </b>
</bible>
```

will be accepted (See [KJV.xml](books/KJV.xml) as an example).

***

### [text2xml.py](text2xml.py)

This file turns any text file arranged in a certain way to an XML file parsable by this program.

You use to program as such:

```
python text2xml.py <inputTextFile> <outputXMLFile> <(optional; defaults to "en") language in Books.json>
```

and the file must be arranged as such:

```
Genesis\t1\t1\tIn the beginning God created the heavens and the earth.
```

*Note*: all the spaces are tabs

## Images

![Photo featuring John 1](docs/bible.png)
![Photo featuring Genesis 1 16](docs/bible2.png)



More stuff coming soon!

## Thanks

Thanks to [OverviewBible](https://overviewbible.com/) for the **BEAUTIFUL** icons!
