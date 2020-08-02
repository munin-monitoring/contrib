.PHONY: lint
lint:
	# TODO: add "--ignore-words .codespell.ignore-words" as soon as travis supports a newer
	#     testing environment (containing codespell 0.11 or later).
	find plugins/ -type f -not -name "*.png" -not -name "*.conf" -not -name "*.jar" -print0 \
		| xargs -0 codespell \
			--exclude-file .codespell.exclude
