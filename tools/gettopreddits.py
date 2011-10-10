"""
This program is free software. It comes without any warranty, to
the extent permitted by applicable law. You can redistribute it
and/or modify it under the terms of the Do What The Fuck You Want
To Public License, Version 2, as published by Sam Hocevar. See
http://sam.zoy.org/wtfpl/COPYING for more details.
"""

from BeautifulSoup import BeautifulSoup
import itertools
import json
import urllib


URL = "http://redditlist.com/nsfw"


def get_text_contents(element):
  ret = []
  _get_text_contents(element, ret)
  return " ".join(ret).strip()


def _get_text_contents(element, output):
  if isinstance(element, basestring):
    output.append(element)
  else:
    for child in element.contents:
      _get_text_contents(child, output)


def main():
  html = urllib.urlopen(URL).read()
  soup = BeautifulSoup(html)

  reddits = []

  for row in soup.findAll("tr"):
    name = row.findChild("td", **{"class": "redditname"})
    subs = row.findChild("td", **{"class": "subs"})

    if name is None or subs is None:
      continue

    reddits.append({
      "name": get_text_contents(name),
      "subs": int(subs.string.replace(",", "")),
    })

  print json.dumps(reddits)


if __name__ == "__main__":
  main()
