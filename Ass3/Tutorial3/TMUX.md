---- TMUX REAMDE -------

Here is the link to a small crash course for tmux : https://robots.thoughtbot.com/a-tmux-crash-course

Mike has provided two bash files:
   do_scenerio1.sh  #Task 1 simulator
   do_scenerio2.sh  #Task 2 simulator

They both require that you have myserver compiled in the directory your running the scripts.
Basically, all you need to do is create your own scenarios (and remember to update your
port number for the curl calls!)


The "tmux" package is installed in linux.csc (but not in ECS 242).
If you have linux, the package is available in the package manager (apt, yum, etc).

In the previous tutorial, I provided a tmux.conf file that allows you to use you
mouse.  Copy it to ~/.tmux.conf to use it (I highly recommend it)


1. Start a new tmux session
tmux new-session -s <session name> -n <session name> -d

Mike uses "csc360" as the session name

2. Split the screen

You can split the screen into multiple panes and set different 
sizes

a) Split horizontally (in half)

tmux split-window -h -t <session name>

This will give you left pane :0.0 and a right pane :0.1

b) Vertical splits (using percentages)

# Split pane :<pane id> vertically 25/75
tmux split-window -t <session name>:<pane id> -v -p 75
tmux split-window -t csc360:0.1 -v -p 75

#Split pane :0.2 (created by above split) vertically 33/66
tmux split-window -t csc360:0.2 -v -p 66

#Split pane :0.3 (created by above split) vertically 50/50 
tmux split-window -t csc360:0.3 -v -p 50

3. You set select a pane to have the curvser 

tmux select-pane -t <session name>:<pane id>

4. To run a command (simulate typing) in a pane your not in

tmux send-keys -t <session name>:<pane id> '/some/command arg1 arg1' C-m

tmux send-keys -t csc360:0.0 './myserver' C-m
sleep 1  #Just type in commands normally for the pane your in
tmux send-keys -t csc360:0.1 'curl "localhost:18300/?op=write&val=june2017"' C-m

5. Not sure what this does ... but Mike always ends his scripts with it

tmux attach -t <session name>

6. To kill the entire session

When your done a session and just want to exist all the panes quickly

tmux kill-session -t <session name>
 
