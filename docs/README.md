# Hydra SDK Documentation

This directory contains the documentation for the Hydra SDK.

## Directory Structure

- `html/`: HTML documentation for viewing in a web browser
  - `index.html`: Main documentation page
  - `modules/`: Documentation for individual modules
- `custom/`: Custom documentation files
  - `overview.md`: Overview of the Hydra SDK
  - `installation.md`: Installation guide
  - `getting_started.md`: Getting started guide
  - `examples.md`: Examples of using the Hydra SDK
  - `advanced_usage.md`: Advanced usage guide
  - `hydra_cli_address.md`: Documentation for the address generation functionality in the Hydra CLI

## Viewing the Documentation

You can view the documentation by opening the `html/index.html` file in a web browser.

## Generating Documentation with GitDoc

The Hydra SDK uses GitDoc for generating documentation from source code. To generate the documentation:

1. Install GitDoc:

```bash
npm install -g gitdoc
```

2. Run GitDoc:

```bash
gitdoc generate
```

This will generate documentation in the `docs/gitdoc` directory based on the configuration in `.gitdoc.yaml`.

## Contributing to the Documentation

To contribute to the documentation:

1. Edit the Markdown files in the `custom/` directory
2. For module-specific documentation, edit the HTML files in the `html/modules/` directory
3. Run GitDoc to regenerate the documentation
4. Submit a pull request with your changes

## Documentation Standards

When contributing to the documentation, please follow these standards:

- Use clear, concise language
- Include examples for all functionality
- Document all parameters and return values
- Include usage examples for both the CLI and API
- Follow the existing documentation structure
- Include diagrams where appropriate

## License

The documentation is licensed under the same license as the Hydra SDK.
