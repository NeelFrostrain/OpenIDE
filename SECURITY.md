# Security Policy

The OpenIDE maintainers take security issues seriously. We appreciate your efforts to responsibly disclose any vulnerabilities you find.

---

## Supported Versions

Only the latest release and active development branch (`main`) receive security updates.

| Version | Supported          |
| ------- | ------------------ |
| 0.1.x   | :white_check_mark: |
| < 0.1.0 | :x:                |

---

## Reporting a Vulnerability

If you discover a security vulnerability within OpenIDE (e.g. arbitrary code execution via malicious workspace settings, path traversal in LSP server handling, or buffer overflows in project indexers), please report it responsibly:

> [!IMPORTANT]
> **DO NOT** report security vulnerabilities through public GitHub Issues or public discussions.

### How to Report

1. **Email**: Send a private report detailing the vulnerability to the project maintainers.
2. **Private Vulnerability Report**: Alternatively, submit a report via GitHub Private Vulnerability Reporting on the repository.

### What to Include in Your Report

To help us triage and fix the vulnerability quickly, please include:

- A clear description of the vulnerability and its potential impact.
- Step-by-step instructions or a Minimal Reproducible Example (PoC).
- Affected version(s) and operating system environment.
- Any suggested mitigations or patches if available.

---

## Response Process & Disclosure Timeline

- **Acknowledgement**: We aim to acknowledge receipt of your security report within **48 hours**.
- **Assessment**: We will evaluate the report, verify the vulnerability, and determine severity.
- **Fix & Advisory**: We will work on a fix in a private branch and coordinate a release date with you.
- **Public Disclosure**: Once a security patch is merged and released, a public security advisory will be published.

Thank you for helping keep OpenIDE and its user community safe!
