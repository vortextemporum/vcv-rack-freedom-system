# /troubleshoot-juce

**Status:** DEPRECATED

---

## Notice

This slash command has been replaced by the `deep-research` Agent Skill.

**Why the change:**
- Skills are invoked autonomously when errors or unknowns are detected
- Integrates with build-automation failure protocol (Option 1: Investigate)
- No need to manually type `/troubleshoot-juce` every time

**To use research now:**
- Say "investigate this" or "research this error"
- Choose "Option 1: Investigate" in build failure protocol
- Claude automatically invokes deep-research skill when appropriate

**This command is kept for backwards compatibility but will be removed in future versions.**

---

## What It Used To Do

Performed thorough research to find authoritative solutions to JUCE plugin development problems.

**Research protocol (now handled by deep-research skill):**
1. Identify the problem (error message, symptom, context)
2. Check local troubleshooting docs first (fastest)
3. Search Context7 for official JUCE documentation
4. Web research if needed (GitHub issues, Stack Overflow, etc.)
5. Cross-reference multiple sources
6. Analyze root cause and solution quality
7. Report findings with confidence level

## Migration Guide

**Old way:**
```
User: /troubleshoot-juce linker error for juce::dsp::Compressor
Assistant: [Manual invocation of research protocol]
          [Searches docs, Context7, web]
          [Reports findings]
```

**New way (automatic):**
```
User: Build failed with linker error for juce::dsp::Compressor
Assistant: Build error detected.

          Options:
          1. Investigate (triggers deep-research)
          2. Show me the code
          3. Show full output
          4. I'll fix it manually

User: [Selects 1]
Assistant: [Automatically invokes deep-research skill]
          [Searches local docs → Context7 → web]
          [Reports findings with solution]
```

**New way (explicit):**
```
User: Investigate this linker error for juce::dsp::Compressor
Assistant: [Detects "investigate" keyword]
          [Automatically invokes deep-research skill]
          [Same research protocol, automatic invocation]
```

## What Replaced It

**Skill:** `deep-research`

**Graduated depth protocol:**
- Level 1 (Fast Path): Search local troubleshooting docs
- Level 2 (Authoritative): Context7 for official JUCE documentation
- Level 3 (Comprehensive): Web research with source evaluation

**Auto-invoked when:**
- Build failures occur (choose "investigate" option)
- Natural language detection ("research this", "investigate this")
- Unknown JUCE APIs encountered
- Complex problems requiring multiple sources

## Why Skills Are Better

**Automatic activation:**
- No need to remember slash command
- Triggers when Claude detects problems
- Integrated into failure protocols

**Contextual awareness:**
- Understands what you're working on
- Searches relevant docs automatically
- Prioritizes local knowledge first

**Seamless integration:**
- Works with build-automation
- Feeds into troubleshooting-docs
- Part of complete problem-solving workflow

## For Historical Reference

**Original research protocol:**
1. Identify problem (error message, context)
2. Check local docs (docs/troubleshooting/)
3. Context7 search (official JUCE docs)
4. Web research (GitHub, Stack Overflow)
5. Cross-reference sources
6. Analyze root cause
7. Report with confidence level

**This protocol still exists, now automated in deep-research skill.**

## Related

- `deep-research` skill - Replacement (automatic invocation)
- `troubleshooting-docs` skill - Creates docs that deep-research searches
- `/doc-fix` command - Document solutions after fixing

## Summary

Don't use this command. Say "investigate this" or choose "Option 1: Investigate" in failure protocols instead. Claude will automatically invoke the deep-research skill with the same research protocol, but better integration and automatic activation