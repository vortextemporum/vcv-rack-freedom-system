## Migration Notes

### What Changes from v1

**Architecture:**
| v1 | v2 | Reason |
|----|-----|--------|
| Monolithic skills | Dispatcher + subagents | Context management |
| No parameter contract | parameter-spec.md required | Eliminate drift |
| No validation | Validator agent | Verify claims |
| Manual quality checks | Hooks | Deterministic enforcement |
| Generate all mockup files | Two-phase design | Faster iteration |

**Workflows:**
| v1 | v2 | Reason |
|----|-----|--------|
| plugin-workflow runs all stages in main context | plugin-workflow dispatches Stages 2-5 to subagents | Fresh context per stage |
| Mockup generates 7 files immediately | Mockup generates 2 files (yaml + test.html), then 7 on finalization | Reduce clutter |
| Stage 1 plans without contracts | Stage 1 blocks if no parameter-spec.md or architecture.md | Enforce contracts |

**State Management:**
| v1 | v2 | Reason |
|----|-----|--------|
| PLUGINS.md updated manually | PLUGINS.md updated by skills (via Claude) | Consistency |
| .continue-here.md optional | .continue-here.md always created | Session continuity |
| No validation of claims | Validator verifies each stage | Catch drift |

### What Stays the Same

**Preserved from v1:**

1. **4-layer architecture** - CLAUDE.md → Skills → Agents → Scripts
2. **WebView UI philosophy** - HTML IS production UI
3. **Graduated research** - troubleshooter 4-level protocol
4. **Build automation** - Never auto-retry, present 4 options
5. **Semantic versioning** - plugin-improve version management
6. **Slash commands** - Explicit workflow triggers
7. **Git integration** - Auto-commits at stage boundaries
8. **Directory structure** - plugins/[Name]/.ideas/ organization

**Why preserve:**
- These patterns worked well in v1
- No pain points identified
- Changing would add complexity without benefit

### How to Port Existing Skills

**Minimal changes needed:**

1. **Add agent definitions** (new in v2):
   - Create `.claude/agents/foundation-agent.md`
   - Create `.claude/agents/shell-agent.md`
   - Create `.claude/agents/dsp-agent.md`
   - Create `.claude/agents/gui-agent.md`

2. **Update plugin-workflow** (modified):
   - Change Stages 2-5 from inline implementation to subagent dispatch
   - Add validator calls after each stage
   - Keep Stages 0, 1, 6 in main context

3. **Update ui-mockup** (modified):
   - Split into two-phase (design iterations vs. finalization)
   - Add parameter-spec.md generation on finalization

4. **Add hooks** (new in v2):
   - Create `.claude/hooks/scripts/juce-validator.py`
   - Create `.claude/hooks/hooks.json`

5. **Other skills** (unchanged):
   - plugin-improve: No changes
   - plugin-ideation: No changes
   - build-automation: No changes
   - All other skills: No changes

**Migration script:**

```bash
#!/bin/bash
# migrate-to-v2.sh

echo "Migrating TÂCHES system from v1 to v2..."

# 1. Create agent directory
mkdir -p .claude/agents

# 2. Copy agent templates from future-system/
cp future-system/agents/* .claude/agents/

# 3. Create hooks directory
mkdir -p .claude/hooks/scripts

# 4. Copy hook scripts
cp future-system/hooks/scripts/* .claude/hooks/scripts/
chmod +x .claude/hooks/scripts/*.py
chmod +x .claude/hooks/scripts/*.sh

# 5. Copy hooks.json
cp future-system/hooks/hooks.json .claude/hooks/

# 6. Update plugin-workflow skill
cp future-system/skills/plugin-workflow/SKILL.md .claude/skills/plugin-workflow/

# 7. Update ui-mockup skill
cp future-system/skills/ui-mockup/SKILL.md .claude/skills/ui-mockup/

echo "✓ Migration complete"
echo ""
echo "Next steps:"
echo "1. Test with new plugin: /dream → /implement"
echo "2. Verify subagents spawn correctly"
echo "3. Check hooks are enforcing quality"
```

**Backward compatibility:**

- v2 system can read v1 plugins (old structure still valid)
- v1 plugins can continue to work alongside v2 plugins
- Gradual migration: Migrate one skill at a time, test, continue

---

## Success Criteria

The architecture document is complete when:

✅ **1. Any developer could implement the system from this design alone**
- All components defined with clear interfaces
- Data flow mapped from request to result
- File structure specified completely
- Discovery and loading mechanisms explained

✅ **2. All component interfaces are precisely defined**
- Skill interface (frontmatter schema)
- Agent interface (input/output schema)
- Validator interface (validation schema)
- Hook interface (exit codes, input/output)

✅ **3. All data flows are mapped with no ambiguity**
- Request → Claude (via CLAUDE.md) → Workflow → Subagent → Validator → Result
- State transitions documented
- Error paths specified
- Critical paths identified

✅ **4. Extension points are clear and documented**
- How to add new skills
- How to add new agents
- How to add new hooks
- How to customize workflows

✅ **5. The design feels cohesive, not incremental**
- Clear core abstractions (5 fundamental concepts)
- Minimal coupling between components
- Predictable behavior
- Clean separation of concerns

✅ **6. WebView integration is first-class, not an afterthought**
- Two-phase UI design workflow
- Parameter contract enforced
- HTML → UI architecture clear
- Resource management specified

✅ **7. The document explains both WHAT and WHY for every decision**
- Design Decisions section explains alternatives considered
- Trade-offs documented
- Rationale provided for each choice

---

## Verification Checklist

Before considering v2 complete, verify:

### Architecture Quality

- ✓ All pain points from v1 addressed (context accumulation, drift, no validation)
- ✓ Clean separation between all components (CLAUDE.md, workflows, subagents, hooks, validator)
- ✓ No circular dependencies (workflows call agents, agents don't call workflows)
- ✓ Clear extension points for future features (new skills, agents, hooks)
- ✓ WebView UI system fully integrated (two-phase design, parameter contract, validation)
- ✓ Every component has a single, clear responsibility
- ✓ Routing logic is deterministic and debuggable (contract-driven, declarative)
- ✓ State management is consistent across all components (single state machine)
- ✓ The architecture could be explained to a new developer in 30 minutes ✓

### Implementation Readiness

- ✓ All interfaces specified (TypeScript/Python schemas)
- ✓ All file paths defined (directory structure complete)
- ✓ All data flows mapped (request → result sequences)
- ✓ All error paths documented (failure handling)
- ✓ All validation points identified (7 validator triggers)
- ✓ All contracts defined (parameter-spec, architecture, plan)
- ✓ All hooks specified (PostToolUse, SessionStart, SubagentStop)

### Cohesiveness

- ✓ Core abstractions are minimal and orthogonal (5 concepts, no overlap)
- ✓ Component relationships are clear (diagram provided)
- ✓ State ownership is unambiguous (ownership table)
- ✓ Extension mechanism is consistent (YAML frontmatter, discovery)
- ✓ Documentation is comprehensive (every section addresses requirements)

**Result:** This design is ready for implementation. Building this would result in a **cohesive, not cobbled-together** system.

---
