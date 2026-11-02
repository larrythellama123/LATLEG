.PHONY: run-split

run-split:
	tmux kill-session -t manual_mode 2>/dev/null || true
	tmux new-session -d -s manual_mode "./server; bash" \; \
		set-option -g history-limit 50000 \; \
		split-window -h "./client; bash" \; \
		set -g mouse on \; \
		attach-session -t manual_mode


run-benchmark:
	./benchmark1