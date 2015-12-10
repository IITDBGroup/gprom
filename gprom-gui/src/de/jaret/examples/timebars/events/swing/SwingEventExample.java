/*
 *  File: SwingEventExample.java 
 *  Copyright (c) 2004-2009  Peter Kliem (Peter.Kliem@jaret.de)
 *  A commercial license is available, see http://www.jaret.de.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
package de.jaret.examples.timebars.events.swing;

import java.awt.BorderLayout;

import javax.swing.JFrame;

import de.jaret.examples.timebars.eventmonitoring.model.ModelCreator;
import de.jaret.examples.timebars.eventmonitoring.swing.renderer.EventRenderer;
import de.jaret.util.date.Interval;
import de.jaret.util.ui.timebars.TimeBarViewerInterface;
import de.jaret.util.ui.timebars.mod.DefaultIntervalModificator;
import de.jaret.util.ui.timebars.model.TimeBarModel;
import de.jaret.util.ui.timebars.swing.TimeBarViewer;

/**
 * Swing: example showing how to render events (i.e. intervals without duration).
 * 
 * @author Peter Kliem
 * @version $Id: SwingTimeBarExample.java 202 2007-01-15 22:00:02Z olk $
 */
public class SwingEventExample {
	static TimeBarViewer _tbv;
	
    public static void main(String[] args) {
    	JFrame f = new JFrame(SwingEventExample.class.getName());
        f.setSize(800, 500);
        f.getContentPane().setLayout(new BorderLayout());
        f.setDefaultCloseOperation(JFrame.DISPOSE_ON_CLOSE);

        TimeBarModel model = ModelCreator.createFlatModel();
        _tbv = new TimeBarViewer(model);

        _tbv.addIntervalModificator(new DefaultIntervalModificator());

        _tbv.setPixelPerSecond(0.05);
        _tbv.setDrawRowGrid(true);
        
        _tbv.setDrawOverlapping(false);
        _tbv.setSelectionDelta(6);
        _tbv.setTimeScalePosition(TimeBarViewerInterface.TIMESCALE_POSITION_TOP);
        
        _tbv.registerTimeBarRenderer((Class<? extends Interval>) SwingEventExample.class, new EventRenderer());
        
        
        f.getContentPane().add(_tbv, BorderLayout.CENTER);

//        f.getContentPane().add(new OverlapControlPanel(_tbv), BorderLayout.SOUTH);
        
        f.setVisible(true);


    }
}
