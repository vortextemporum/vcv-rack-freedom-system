# Success Menu Templates

## Template Structure

```
{{COMPLETION_MESSAGE}}. What's next?
{{#OPTIONS}}
{{INDEX}}. {{LABEL}}{{#RECOMMENDED}} (recommended){{/RECOMMENDED}}
{{/OPTIONS}}
```

## Menu Definitions

### Stage 1 (Foundation)
**Completion message:** "Foundation verified"
**Options:**
1. Continue to Stage 2 (Shell/Parameters) [recommended]
2. Review generated code
3. Pause workflow

### Stage 2 (Shell)
**Completion message:** "Shell built successfully"
**Options:**
1. Continue to Stage 3 (DSP) [recommended]
2. Test in DAW now
3. Review parameter code
4. Pause workflow

### Stage 3 (DSP)
**Completion message:** "DSP implementation complete"
**Options:**
1. Continue to Stage 4 (GUI) [recommended]
2. Test in DAW now
3. Review DSP code
4. Pause workflow

### Stage 4 (GUI)
**Completion message:** "GUI implementation complete"
**Options:**
1. Continue to Stage 4 (Validation) [recommended]
2. Test in DAW now
3. Review GUI code
4. Pause workflow

### Stage 4 (Validation)
**Completion message:** "Plugin complete and validated"
**Options:**
1. Test in DAW now [recommended]
2. Review final code
3. Package for distribution
4. Start new plugin

### plugin-improve
**Completion message:** "Improvements applied successfully"
**Options:**
1. Test in DAW now [recommended]
2. Review changes
3. Apply more improvements
4. Finalize version

## Usage

Load appropriate template based on invoking context:
- Extract stage from invocation parameters
- Select matching template
- Substitute {{COMPLETION_MESSAGE}} and {{OPTIONS}}
- Present to user
