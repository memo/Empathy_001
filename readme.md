# Realism, abstraction and empathy in video game violence 
Video playback and data capture app for

***Realism, abstraction and empathy in video game violence*** study

Copyright © 2014, Memo Akten



# Dependencies
Written in C++ built on [openFrameworks](https://github.com/openframeworks/openframeworks)
(current version built with v0.8.4)

Should work on any system that supports openFrameworks, but tested on OSX 10.9.5

The videos have been omitted from this repo as they were very large. The two videos are loaded from:

- bin/data/videos/test.mp4	// the test video triggered when 't' is pressed
- bin/data/videos/Empathy_001.mov	// the main video

**Update**: Uses CGWarpMouseCursorPosition to set mouse position (to reset mouse to center before data capture). This will only work on OSX. This is not critical behaviour so can be commented out.



# License
This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.