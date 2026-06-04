.PHONY: all cmake clearall help

BUILD_DIR ?= build

# Явные независимые таргеты
cmake:
	cmake -B $(BUILD_DIR)

# Внутренние таргеты для проверок
.interactive-confirm:
	@if [ "$(FORCE)" != "1" ]; then \
		echo -n "Are you sure you want to clean $(BUILD_DIR)? [y/N] "; \
		read ans; \
		if [ "$$ans" != "y" ] && [ "$$ans" != "Y" ]; then \
			exit 0; \
		fi; \
	fi

.check-not-current-dir:
	@if [ "$$(realpath "$(BUILD_DIR)")" = "$$(realpath .)" ]; then \
		echo "Error: Cannot clean current directory"; \
		exit 1; \
	fi

# Таргет clearall с зависимостями на проверки
clearall: .check-not-current-dir .interactive-confirm
	@rm -rf $(BUILD_DIR)

help:
	@echo "Available targets: cmake, clearall, and any cmake build target"
	@echo ""
	@echo "  BUILD_DIR=<dir> - build directory flag (applies to all targets, default: build)"
	@echo "  Usage: make BUILD_DIR=<dir> <target>"
	@echo ""
	@echo "  make clearall              - interactive (prompts for confirmation)"
	@echo "  make clearall FORCE=1      - skip confirmation prompt"
	@echo "  make cmake                 - generate build system"
	@echo "  make <other>               - pass through to cmake build (e.g., make build, make install)"

# Таргет для создания build-директории
$(BUILD_DIR):
	$(MAKE) cmake

# Паттерн-правило для всех остальных команд
%:
	@if [ ! -d $(BUILD_DIR) ]; then $(MAKE) $(BUILD_DIR); fi
	$(MAKE) -C $(BUILD_DIR) $@
