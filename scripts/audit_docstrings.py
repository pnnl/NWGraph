#!/usr/bin/env python3
"""
Docstring Audit Script for NWGraph

Analyzes header files to identify documentation gaps:
- Files missing @brief descriptions
- Functions/classes missing @param/@tparam
- Functions missing @return
- Files without @example blocks

Usage:
    python scripts/audit_docstrings.py [--verbose] [--file FILE]
"""

import argparse
import os
import re
import sys
from pathlib import Path
from dataclasses import dataclass, field
from typing import Optional

@dataclass
class DocIssue:
    """A documentation issue found in a file."""
    file: str
    line: int
    issue_type: str
    entity_name: str
    message: str
    severity: str = "warning"  # "error", "warning", "info"

@dataclass
class FileAudit:
    """Audit results for a single file."""
    path: str
    has_file_brief: bool = False
    has_examples: bool = False
    function_count: int = 0
    documented_functions: int = 0
    class_count: int = 0
    documented_classes: int = 0
    concept_count: int = 0
    documented_concepts: int = 0
    issues: list = field(default_factory=list)

def find_header_files(base_dir: str, exclude_dirs: list = None) -> list:
    """Find all .hpp files, excluding specified directories."""
    exclude_dirs = exclude_dirs or ['experimental']
    headers = []
    for root, dirs, files in os.walk(base_dir):
        # Filter out excluded directories
        dirs[:] = [d for d in dirs if d not in exclude_dirs]
        for f in files:
            if f.endswith('.hpp'):
                headers.append(os.path.join(root, f))
    return sorted(headers)

def extract_doxygen_comment(lines: list, end_line: int) -> Optional[tuple]:
    """
    Extract the Doxygen comment block ending at or before end_line.
    Returns (comment_text, start_line, end_line) or None.
    """
    # Look backwards for a comment block
    i = end_line - 1
    while i >= 0 and lines[i].strip() == '':
        i -= 1

    if i < 0:
        return None

    # Check for single-line /// comment or multi-line /** */ block
    comment_end = i
    comment_lines = []

    # Handle /// style comments
    while i >= 0 and lines[i].strip().startswith('///'):
        comment_lines.insert(0, lines[i])
        i -= 1

    if comment_lines:
        return ('\n'.join(comment_lines), i + 1, comment_end)

    # Handle /** */ style comments
    if '*/' in lines[comment_end]:
        # Find the start of the block
        while i >= 0:
            if '/**' in lines[i]:
                for j in range(i, comment_end + 1):
                    comment_lines.append(lines[j])
                return ('\n'.join(comment_lines), i, comment_end)
            i -= 1

    return None

def parse_comment_tags(comment: str) -> dict:
    """Parse Doxygen tags from a comment block."""
    tags = {
        'brief': [],
        'param': [],
        'tparam': [],
        'return': [],
        'example': [],
        'note': [],
        'see': [],
    }

    if not comment:
        return tags

    # Extract @brief or \brief
    brief_match = re.search(r'[@\\]brief\s+(.+?)(?=[@\\](?:param|tparam|return|example|note|see|code)|$)',
                           comment, re.DOTALL)
    if brief_match:
        tags['brief'].append(brief_match.group(1).strip())

    # Extract @param entries
    for match in re.finditer(r'[@\\]param\s+(\w+)\s+(.+?)(?=[@\\](?:param|tparam|return|example)|$)',
                             comment, re.DOTALL):
        tags['param'].append((match.group(1), match.group(2).strip()))

    # Extract @tparam entries
    for match in re.finditer(r'[@\\]tparam\s+(\w+)\s+(.+?)(?=[@\\](?:param|tparam|return|example)|$)',
                             comment, re.DOTALL):
        tags['tparam'].append((match.group(1), match.group(2).strip()))

    # Extract @return
    return_match = re.search(r'[@\\]return\s+(.+?)(?=[@\\](?:param|tparam|example|note|see)|$)',
                            comment, re.DOTALL)
    if return_match:
        tags['return'].append(return_match.group(1).strip())

    # Check for @example
    if re.search(r'[@\\]example', comment):
        tags['example'].append(True)

    return tags

def extract_template_params(declaration: str) -> list:
    """Extract template parameter names from a template declaration."""
    params = []
    # Match template<...>
    match = re.search(r'template\s*<([^>]+)>', declaration)
    if match:
        template_content = match.group(1)
        # Parse individual parameters
        # Handle typename X, class Y, auto Z, concept C, etc.
        for param in re.finditer(r'(?:typename|class|auto|\w+)\s+(\w+)(?:\s*=|,|$)', template_content):
            params.append(param.group(1))
        # Also handle non-type template params like size_t N
        for param in re.finditer(r'(?:size_t|int|bool|auto)\s+(\w+)', template_content):
            if param.group(1) not in params:
                params.append(param.group(1))
    return params

def extract_function_params(declaration: str) -> list:
    """Extract parameter names from a function declaration."""
    params = []
    # Find the parameter list between ( and )
    # Handle multi-line by looking for balanced parens
    paren_start = declaration.find('(')
    if paren_start == -1:
        return params

    depth = 1
    i = paren_start + 1
    param_content = ""
    while i < len(declaration) and depth > 0:
        if declaration[i] == '(':
            depth += 1
        elif declaration[i] == ')':
            depth -= 1
        if depth > 0:
            param_content += declaration[i]
        i += 1

    # Parse parameter names - look for "type name" or "type name = default"
    # Split by comma, handling nested templates
    current_param = ""
    depth = 0
    for char in param_content:
        if char in '<(':
            depth += 1
        elif char in '>)':
            depth -= 1
        elif char == ',' and depth == 0:
            # Extract param name from current_param
            name = extract_param_name(current_param.strip())
            if name:
                params.append(name)
            current_param = ""
            continue
        current_param += char

    # Handle last parameter
    if current_param.strip():
        name = extract_param_name(current_param.strip())
        if name:
            params.append(name)

    return params

def extract_param_name(param_decl: str) -> Optional[str]:
    """Extract the parameter name from a parameter declaration."""
    if not param_decl:
        return None

    # Remove default value
    param_decl = re.sub(r'\s*=\s*.+$', '', param_decl)

    # Handle const&, &&, *, etc.
    param_decl = param_decl.strip()

    # Extract the last word that looks like an identifier
    match = re.search(r'(\w+)\s*$', param_decl)
    if match:
        name = match.group(1)
        # Filter out type-like names
        if name not in ['const', 'auto', 'void', 'int', 'size_t', 'bool', 'float', 'double']:
            return name
    return None

def audit_file(filepath: str) -> FileAudit:
    """Audit a single header file for documentation issues."""
    audit = FileAudit(path=filepath)

    try:
        with open(filepath, 'r', encoding='utf-8', errors='replace') as f:
            content = f.read()
            lines = content.split('\n')
    except Exception as e:
        audit.issues.append(DocIssue(
            file=filepath, line=0, issue_type="read_error",
            entity_name="", message=f"Could not read file: {e}", severity="error"
        ))
        return audit

    # Check for file-level @brief or @file
    if re.search(r'[@\\](?:brief|file)\s+\w', content[:2000]):
        audit.has_file_brief = True
    else:
        audit.issues.append(DocIssue(
            file=filepath, line=1, issue_type="missing_file_brief",
            entity_name=os.path.basename(filepath),
            message="File lacks @file or @brief documentation",
            severity="warning"
        ))

    # Check for @example blocks
    if re.search(r'[@\\]example', content):
        audit.has_examples = True

    # Find and audit functions
    # Pattern for function declarations (simplified)
    func_pattern = re.compile(
        r'^(?:template\s*<[^>]+>\s*)?\s*'  # optional template
        r'(?:inline\s+|static\s+|constexpr\s+|auto\s+)*'  # modifiers
        r'(?:[\w:]+(?:<[^>]*>)?\s+)?'  # return type (optional for constructors)
        r'(\w+)\s*'  # function name
        r'\([^)]*\)',  # parameters
        re.MULTILINE
    )

    # Find concepts
    concept_pattern = re.compile(r'^\s*template\s*<[^>]+>\s*concept\s+(\w+)\s*=', re.MULTILINE)
    for match in concept_pattern.finditer(content):
        audit.concept_count += 1
        concept_name = match.group(1)
        line_num = content[:match.start()].count('\n') + 1

        comment_result = extract_doxygen_comment(lines, line_num - 1)
        if comment_result:
            comment_text, _, _ = comment_result
            tags = parse_comment_tags(comment_text)
            if tags['brief']:
                audit.documented_concepts += 1
            else:
                audit.issues.append(DocIssue(
                    file=filepath, line=line_num, issue_type="missing_brief",
                    entity_name=concept_name,
                    message=f"Concept '{concept_name}' lacks @brief description",
                    severity="warning"
                ))
        else:
            audit.issues.append(DocIssue(
                file=filepath, line=line_num, issue_type="undocumented",
                entity_name=concept_name,
                message=f"Concept '{concept_name}' has no documentation",
                severity="warning"
            ))

    # Find classes/structs
    class_pattern = re.compile(
        r'^\s*(?:template\s*<[^>]+>\s*)?\s*(?:class|struct)\s+(\w+)',
        re.MULTILINE
    )
    for match in class_pattern.finditer(content):
        # Skip forward declarations
        rest_of_line = content[match.end():match.end()+50]
        if ';' in rest_of_line.split('\n')[0] and '{' not in rest_of_line.split('\n')[0]:
            continue

        audit.class_count += 1
        class_name = match.group(1)
        line_num = content[:match.start()].count('\n') + 1

        comment_result = extract_doxygen_comment(lines, line_num - 1)
        if comment_result:
            comment_text, _, _ = comment_result
            tags = parse_comment_tags(comment_text)
            if tags['brief']:
                audit.documented_classes += 1
            else:
                audit.issues.append(DocIssue(
                    file=filepath, line=line_num, issue_type="missing_brief",
                    entity_name=class_name,
                    message=f"Class '{class_name}' lacks @brief description",
                    severity="info"
                ))
        else:
            audit.issues.append(DocIssue(
                file=filepath, line=line_num, issue_type="undocumented",
                entity_name=class_name,
                message=f"Class '{class_name}' has no documentation",
                severity="info"
            ))

    # Find standalone functions (not inside classes)
    # This is a simplified check - just looks for function-like patterns
    standalone_func_pattern = re.compile(
        r'^(?:template\s*<[^>]+>\s*\n?)?\s*'
        r'(?:inline\s+|static\s+|constexpr\s+)*'
        r'(?:auto|void|bool|int|size_t|[\w:]+(?:<[^>]*>)?)\s+'
        r'(\w+)\s*\(',
        re.MULTILINE
    )

    for match in standalone_func_pattern.finditer(content):
        func_name = match.group(1)
        # Skip common non-function patterns
        if func_name in ['if', 'for', 'while', 'switch', 'return', 'sizeof', 'alignof', 'decltype']:
            continue
        # Skip things that look like constructor calls or variable declarations
        line_start = content.rfind('\n', 0, match.start()) + 1
        line = content[line_start:match.end()]
        if line.strip().startswith('//') or line.strip().startswith('/*'):
            continue

        audit.function_count += 1
        line_num = content[:match.start()].count('\n') + 1

        comment_result = extract_doxygen_comment(lines, line_num - 1)
        if comment_result:
            comment_text, _, _ = comment_result
            tags = parse_comment_tags(comment_text)
            if tags['brief']:
                audit.documented_functions += 1
        # Note: We don't report every undocumented function to avoid noise

    return audit

def generate_report(audits: list, verbose: bool = False) -> str:
    """Generate a summary report of the audit results."""
    lines = []
    lines.append("=" * 70)
    lines.append("NWGraph Docstring Audit Report")
    lines.append("=" * 70)
    lines.append("")

    # Summary statistics
    total_files = len(audits)
    files_with_brief = sum(1 for a in audits if a.has_file_brief)
    files_with_examples = sum(1 for a in audits if a.has_examples)
    total_concepts = sum(a.concept_count for a in audits)
    documented_concepts = sum(a.documented_concepts for a in audits)
    total_classes = sum(a.class_count for a in audits)
    documented_classes = sum(a.documented_classes for a in audits)
    total_functions = sum(a.function_count for a in audits)
    documented_functions = sum(a.documented_functions for a in audits)

    lines.append("SUMMARY")
    lines.append("-" * 40)
    lines.append(f"Total header files: {total_files}")
    lines.append(f"Files with @file/@brief: {files_with_brief} ({100*files_with_brief//total_files}%)")
    lines.append(f"Files with @example: {files_with_examples} ({100*files_with_examples//total_files}%)")
    lines.append("")
    lines.append(f"Concepts: {documented_concepts}/{total_concepts} documented")
    lines.append(f"Classes/Structs: {documented_classes}/{total_classes} documented")
    lines.append(f"Functions (est.): {documented_functions}/{total_functions} documented")
    lines.append("")

    # Categorize files by documentation quality
    well_documented = []
    needs_work = []
    undocumented = []

    for audit in audits:
        rel_path = os.path.relpath(audit.path)
        if audit.has_file_brief and len(audit.issues) <= 2:
            well_documented.append(rel_path)
        elif audit.has_file_brief or audit.documented_concepts > 0 or audit.documented_classes > 0:
            needs_work.append(rel_path)
        else:
            undocumented.append(rel_path)

    lines.append("FILES BY DOCUMENTATION STATUS")
    lines.append("-" * 40)
    lines.append(f"\n✅ Well documented ({len(well_documented)} files):")
    for f in sorted(well_documented):
        lines.append(f"   {f}")

    lines.append(f"\n⚠️  Needs work ({len(needs_work)} files):")
    for f in sorted(needs_work):
        lines.append(f"   {f}")

    lines.append(f"\n❌ Undocumented ({len(undocumented)} files):")
    for f in sorted(undocumented):
        lines.append(f"   {f}")

    if verbose:
        lines.append("")
        lines.append("DETAILED ISSUES")
        lines.append("-" * 40)
        for audit in audits:
            if audit.issues:
                rel_path = os.path.relpath(audit.path)
                lines.append(f"\n{rel_path}:")
                for issue in audit.issues:
                    severity_icon = {"error": "❌", "warning": "⚠️", "info": "ℹ️"}.get(issue.severity, "•")
                    lines.append(f"  {severity_icon} Line {issue.line}: {issue.message}")

    lines.append("")
    lines.append("=" * 70)
    lines.append("End of Report")
    lines.append("=" * 70)

    return '\n'.join(lines)

def main():
    parser = argparse.ArgumentParser(description="Audit NWGraph header docstrings")
    parser.add_argument('--verbose', '-v', action='store_true',
                       help="Show detailed issues for each file")
    parser.add_argument('--file', '-f', type=str,
                       help="Audit a specific file only")
    parser.add_argument('--json', action='store_true',
                       help="Output results as JSON")
    args = parser.parse_args()

    # Find the project root
    script_dir = Path(__file__).parent
    project_root = script_dir.parent
    include_dir = project_root / "include" / "nwgraph"

    if args.file:
        headers = [args.file]
    else:
        headers = find_header_files(str(include_dir), exclude_dirs=['experimental'])

    print(f"Auditing {len(headers)} header files...", file=sys.stderr)

    audits = []
    for header in headers:
        audit = audit_file(header)
        audits.append(audit)

    if args.json:
        import json
        result = {
            'summary': {
                'total_files': len(audits),
                'files_with_brief': sum(1 for a in audits if a.has_file_brief),
                'files_with_examples': sum(1 for a in audits if a.has_examples),
            },
            'files': [
                {
                    'path': os.path.relpath(a.path),
                    'has_file_brief': a.has_file_brief,
                    'has_examples': a.has_examples,
                    'concepts': a.concept_count,
                    'documented_concepts': a.documented_concepts,
                    'classes': a.class_count,
                    'documented_classes': a.documented_classes,
                    'issues': [
                        {
                            'line': i.line,
                            'type': i.issue_type,
                            'entity': i.entity_name,
                            'message': i.message,
                            'severity': i.severity,
                        }
                        for i in a.issues
                    ]
                }
                for a in audits
            ]
        }
        print(json.dumps(result, indent=2))
    else:
        print(generate_report(audits, verbose=args.verbose))

if __name__ == '__main__':
    main()
