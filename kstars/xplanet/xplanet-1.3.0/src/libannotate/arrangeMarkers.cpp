#include <algorithm>
#include <map>
using namespace std;

#include "keywords.h"

#include "libannotate/Text.h"
#include "libdisplay/libdisplay.h"

static int
findOverlap(multimap<int, Text *> &textMap, Text *text)
{
    int totalOverlap = 0;
    multimap<int, Text *>::iterator textIterator;
    for (textIterator = textMap.begin(); 
	 textIterator != textMap.end(); 
	 textIterator++)
    {
	Text *t = textIterator->second;
	if (t != text) totalOverlap += text->Overlap(t);
    }
    return(totalOverlap);
}

void
arrangeMarkers(multimap<double, Annotation *> &annotationMap,
	       DisplayBase *display)
{
    if (annotationMap.empty()) return;

    // This will hold a list of text strings, sorted by x coordinate
    multimap<int, Text *> textMap;

    multimap<double, Annotation *>::iterator annotationIterator;
    for (annotationIterator = annotationMap.begin(); 
	 annotationIterator != annotationMap.end(); 
	 annotationIterator++)
    {
	Text *t = dynamic_cast<Text *> (annotationIterator->second);
	if (t != NULL)
	{
	    t->ComputeBoundingBox(display);
	    textMap.insert(pair<const int, Text *>(t->X(), t));
	}
    }

    const int align[4] = { RIGHT, LEFT, ABOVE, BELOW };

    for (int i = 0; i < 2; i++)
    {
	multimap<int, Text *>::iterator textIterator;
	for (textIterator = textMap.begin(); 
	     textIterator != textMap.end(); 
	     textIterator++)
	{
	    Text *t = textIterator->second;

	    if (t->FixedAlign()) continue;

	    int totalOverlap = 0;
	    int minOverlap = 0;
	    int alignIndex = 0;

	    // Choose the alignment which yields the minimum overlap for
	    // this marker
	    for (int i = 0; i < 4; i++)
	    {
		if (i == 0 || totalOverlap)
		{
		    t->Align(align[i]);

		    totalOverlap = findOverlap(textMap, t);
		    totalOverlap += t->Overhang(display->Width(), 
						display->Height());
		    if (i == 0 || totalOverlap < minOverlap)
		    {
			minOverlap = totalOverlap;
			alignIndex = i;
		    }
		}
	    }
	    t->Align(align[alignIndex]);
	}    
    }
}
