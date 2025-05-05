#pragma once

#include <vector>
#include <string>

namespace hydra {
namespace cli {

/**
 * Base class for all CLI commands
 */
class Command {
public:
    virtual ~Command() = default;
    
    /**
     * Execute the command with the given arguments
     * 
     * @param args Command arguments
     * @return Exit code (0 for success, non-zero for error)
     */
    virtual int execute(const std::vector<std::string>& args) = 0;
    
    /**
     * Print help information for this command
     */
    virtual void print_help() const = 0;
    
protected:
    /**
     * Check if help flag is present in arguments
     */
    bool is_help_requested(const std::vector<std::string>& args) const {
        for (const auto& arg : args) {
            if (arg == "--help" || arg == "-h") {
                return true;
            }
        }
        return false;
    }
    
    /**
     * Extract the value of a named parameter
     * 
     * @param args Arguments vector
     * @param param Parameter name (e.g., "--password")
     * @param default_value Default value if parameter is not found
     * @return Parameter value or default value
     */
    std::string get_param_value(
        const std::vector<std::string>& args,
        const std::string& param,
        const std::string& default_value = ""
    ) const {
        for (size_t i = 0; i < args.size() - 1; ++i) {
            if (args[i] == param) {
                return args[i + 1];
            }
        }
        return default_value;
    }
    
    /**
     * Check if a flag is present in arguments
     * 
     * @param args Arguments vector
     * @param flag Flag to check for (e.g., "--force")
     * @return True if flag is present
     */
    bool has_flag(const std::vector<std::string>& args, const std::string& flag) const {
        for (const auto& arg : args) {
            if (arg == flag) {
                return true;
            }
        }
        return false;
    }
};

} // namespace cli
} // namespace hydra
