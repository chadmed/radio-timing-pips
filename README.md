# Radio Timing Pip Player

This is the ~~absolute~~ second worst way to do this.

## What?
The Greenwich Time Signal is a historically significant standard in radio broadcasting,
particularly in countries that were/are members of the Commonwealth.
At the top of every hour, a series of 6 "pips" play - one for each of the five
seconds preceding the hour, and one at *exactly* the top of the hour. After these pips, a
customary period of silence lasting half a second is usually followed by the station or network's
fanfare, then the hourly news. Among the important uses of the time signal are:
* Synchronising ship and aircraft chronometers
* Setting your watch
* Cuing people to shut up and get ready for the news

In the UK, the pips are a 1 kHz pure sine wave lasting a quarter of a second. The final
pip at the top of the hour has a duration of half a second. Curiously, all
Australian radio stations which broadcast a time signal historically used a 735 Hz
sine wave instead, with all pips lasting half a second.

You will notice that I said Australia use**d** a 735 Hz time signal. In 2023, the Australian
Broadcasting Corporation's ABC Sydney station (702 kHz AM) became the last station in the ABC
network to cease the broadcast of time pips. The ABC cited the ongoing cost of maintaining an
atomic clock and sine wave generator from the 1940s, and the technical infeasibility of replacing them
with something else. The final use of the timing signal was 2023-11-23T08:00:00Z (19:00:00L).

Interestingly, all ABC stations still maintain radio silence in the 6.5 seconds where the timing signal
used to be.

## Why?
Broadcast and entertainment are just about the only technical sectors where tradition remains a decently strong
influence on technology and practices. While the timing signal is probably not useful for most going
about their GPS-and-NTP-enabled modern lives, it has been a stable of radio broadcast for 100 years. Getting rid of it has left
an audible hole in the broadcast.

The ABC's claims that it would be expensive and infeasible to replace the ancient atomic clock and
pre-war sine wave generator are complete nonsense. I wrote this program in less than an hour.

You really only need two things to broadcast a time signal:
* A certifiably accurate clock
* A signal to broadcast

I am choosing to ignore digital radio because it sucks.

If any ABC tehcnical staff stuble upon this, feel free to wire up a GPS clock to an SBC's RTC header, load up the Majestic Fanfare,
and throw it in a cupboard.
