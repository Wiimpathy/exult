/** 
 **	playfli.cc - Play Autodesk Animator FLIs
 **
 **	Written: 5/5/2000 - TST
 **/

/*
Copyright (C) 2000  Tristan Tarrant

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include <string.h>
#include <iostream.h>
#include "databuf.h"
#include "playfli.h"
#include "utils.h"

playfli::playfli(const char *fli_name)
{
    ifstream fli_stream; 
    U7open(fli_stream, fli_name);
    fli_data = new StreamDataSource(&fli_stream);

    initfli();
}

playfli::playfli(char *buffer, size_t len): fli_data(new BufferDataSource(buffer, len))
{
    initfli();
}

void playfli::initfli()
{
    fli_size = fli_data->read4();
    fli_magic = fli_data->read2();
    fli_frames = fli_data->read2();
    fli_width = fli_data->read2();
    fli_height = fli_data->read2();
    fli_depth = fli_data->read2();
    fli_flags = fli_data->read2();
    fli_speed = fli_data->read2();
    fli_data->skip(110);
    streampos = streamstart = fli_data->getPos();
    frame = 0;
}

void playfli::info(fliinfo *fi)
{
    cout << "Frame count : " << fli_frames << endl;
    cout << "Width :       " << fli_width << endl;
    cout << "Height :      " << fli_height << endl;
    cout << "Depth :       " << fli_depth << endl;
    cout << "Speed :       " << fli_speed << endl;
    
    if (fi)
    {
    	fi->frames = fli_frames;
    	fi->width = fli_width;
    	fi->height = fli_height;
    	fi->depth = fli_depth;
    	fi->speed = fli_speed;
    }
}

// How to use th
//
//
int playfli::play(Image_window *win, int first_frame, int last_frame, unsigned long ticks, int brightness)
{
    int frame_size;
    int frame_magic;
    int frame_chunks;
    int chunk_size;
    int chunk_type;
    unsigned char *pixbuf;
    int xoffset=(win->get_width()-fli_width)/2;
    int yoffset=(win->get_height()-fli_height)/2;
    bool dont_show = false;

    // Set up last frame
    if (first_frame == last_frame) dont_show = true;
    if (first_frame < 0) first_frame += fli_frames;
    if (last_frame < 0) last_frame += 1 + fli_frames;
    if (first_frame == last_frame) last_frame++;
    if (last_frame < 0 || last_frame > fli_frames) last_frame = fli_frames;

    if (!ticks) ticks = SDL_GetTicks();

    if (first_frame < frame)
    {
    	    frame = 0;
	    streampos = streamstart;
    }
    pixbuf = new unsigned char[fli_width];
    // Play frames...
    for ( ; frame < last_frame; frame++)
      {
	  fli_data->seek(streampos);
	  frame_size = fli_data->read4();
	  frame_magic = fli_data->read2();
	  frame_chunks = fli_data->read2();
          fli_data->skip(8);
	  for (int chunk = 0; chunk < frame_chunks; chunk++)
	    {
		chunk_size = fli_data->read4();
		chunk_type = fli_data->read2();

		switch (chunk_type)
		  {
		  case 11:
		      {
			  int packets = fli_data->read2();
			  unsigned char colors[3 * 256];

			  memset(colors, 0, 3 * 256);
			  int current = 0;

			  for (int p_count = 0; p_count < packets;
			       p_count++)
			    {
				int skip = fli_data->read1();

				current += skip;
				int change = fli_data->read1();

				if (change == 0)
				    change = 256;
				fli_data->read((char*)&colors[current*3], change*3);
			    }
			  // Set palette
			  if(win)
				win->set_palette(colors, 64, brightness);
		      }
		      break;
		  case 12:
		      {
			  int skip_lines = fli_data->read2();
			  int change_lines = fli_data->read2();
			  for (int line = 0; line < change_lines; line++)
			    {
				int packets = fli_data->read1();
				int pixpos = xoffset;
				for (int p_count = 0; p_count < packets;
				     p_count++)
				  {
				      int skip_count = fli_data->read1();
				      pixpos += skip_count;
				      char size_count = fli_data->read1();
				      if (size_count < 0)
					{
					    unsigned char data = fli_data->read1();
					    memset(pixbuf, data, -size_count);
					    win->copy8(pixbuf,-size_count,1,pixpos,skip_lines+line+yoffset);
					    pixpos -= size_count;
					    
				        } else {
					    fli_data->read((char*)pixbuf, size_count);
					    win->copy8(pixbuf,size_count,1,pixpos, skip_lines+line+yoffset);
					    pixpos += size_count;
					}
				  }
			    }
		      }
		      break;
		  case 13:
		      break;
		  case 15:
		      for (int line = 0; line < fli_height; line++)
			{
			    int packets = fli_data->read1();
			    int pixpos = 0;
			    for (int p_count = 0; p_count < packets;
				 p_count++)
			      {
				  char size_count = fli_data->read1();
				  if (size_count > 0)
				    {
					unsigned char data = fli_data->read1();
					memset(&pixbuf[pixpos], data, size_count);
					pixpos += size_count;
				    } else {
					fli_data->read((char*)&pixbuf[pixpos],
							-size_count);
					pixpos -= size_count;
				    }
			      }
				if(win && frame >= first_frame)
                                   win->copy8(pixbuf,fli_width,1,xoffset,line+yoffset);
			}
		      break;
		  case 16:
		      fli_data->skip(fli_width * fli_height);
		      break;
		  default:
		      cout << "UNKNOWN FLIC FRAME" << endl;
		      break;
		  }
	    }

	  streampos += frame_size;
	  
	  if (frame < first_frame)
		  continue;

	  while (SDL_GetTicks() < ticks)
	  	;
	  ticks += fli_speed*10;

	  if(win && !dont_show)
             win->show();
      }
    delete[] pixbuf;
	
  return ticks;
}

playfli::~playfli()
{
	delete fli_data;
}
