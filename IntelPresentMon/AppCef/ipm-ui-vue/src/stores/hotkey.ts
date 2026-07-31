import { ref, reactive, readonly, toRaw } from 'vue'
import { defineStore } from 'pinia'
import type { ModifierOption, KeyOption, Binding, KeyCode, ModifierCode } from '@/core/hotkey'
import { Action as HotkeyAction } from '@/core/hotkey'
import { Api } from '@/core/api'
import { getEnumValues } from '@/core/meta'
import { deepToRaw } from '@/core/vue-utils'
import { useNotificationsStore } from './notifications'
import { zhCN } from '@/locales/zh-CN'

function getHotkeyActionName(action: HotkeyAction): string {
    switch (action) {
        case HotkeyAction.ToggleCapture:
            return zhCN.hotkey.toggleCapture
        case HotkeyAction.ToggleOverlay:
            return zhCN.hotkey.toggleOverlay
        case HotkeyAction.CyclePreset:
            return zhCN.hotkey.cyclePreset
        case HotkeyAction.ToggleEtlLogging:
            return zhCN.hotkey.toggleEtlLogging
        default:
            return HotkeyAction[action] ?? ''
    }
}

export const useHotkeyStore = defineStore('hotkey', () => {
    // === Dependent Stores ===
    const notes = useNotificationsStore()
    // === State ===
    const keyOptions = ref<KeyOption[]>([])
    const modifierOptions = ref<ModifierOption[]>([])
    const bindings = reactive<{ [key: string]: Binding }>(
        // add an entry for each action in the (Hotkey)Action enum
        Object.fromEntries(
            getEnumValues(HotkeyAction).map((action) => [
                HotkeyAction[action],
                { action, combination: null },
            ])
        )
    )
    // TODO: receive this from kernel or generate based on SSoT
    const defaultBindings = [
        {
          action: HotkeyAction.ToggleCapture,
          combination: { key: 42 as KeyCode, modifiers: [2 as ModifierCode, 4 as ModifierCode] },
        },
        {
          action: HotkeyAction.CyclePreset,
          combination: { key: 47 as KeyCode, modifiers: [2 as ModifierCode, 4 as ModifierCode] },
        },
        {
          action: HotkeyAction.ToggleOverlay,
          combination: { key: 46 as KeyCode, modifiers: [2 as ModifierCode, 4 as ModifierCode] },
        },
    ] as Binding[]

    // === Actions ===
    // use the api to get the keys/modifiers from the kernel
    async function refreshOptions() {
        keyOptions.value = await Api.enumerateKeys()
        modifierOptions.value = await Api.enumerateModifiers()
    }
    // bind default combinations to the actions
    async function bindDefaults() {
      for (const binding of defaultBindings) {
        try {
          await bindHotkey(binding)
        } catch (e) {
          notes.notify({ text: zhCN.hotkey.bindDefaultFailed(getHotkeyActionName(binding.action)) })
          console.error([`Unable to bind default hotkey for ${HotkeyAction[binding.action]}`, e])
        }
      }
    }
    // set a hotkey combination for an action
    async function bindHotkey(binding: Binding) {
      try {
        bindings[HotkeyAction[binding.action]] = binding
        // we need to clone the binding because pinia turns array into an object
        await Api.bindHotkey(deepToRaw(binding))
      } catch (e) {
        const actionName = HotkeyAction[binding.action]
        notes.notify({ text: zhCN.hotkey.bindFailed(getHotkeyActionName(binding.action)) })
        console.error([`Failed to bind hotkey; Action: [${actionName}]`, e])
      }
    }
    // clear the hotkey combination for an action (make it unbound)
    async function clearHotkey(action: HotkeyAction) {
      try {
        await Api.clearHotkey(action)
        bindings[HotkeyAction[action]] = { action, combination: null }
      } catch (e) {
        const actionName = HotkeyAction[action]
        notes.notify({ text: zhCN.hotkey.clearFailed(getHotkeyActionName(action)) })
        console.error([`Failed to clear hotkey; Action: [${actionName}]`, e])
      }
    }

    // === Exports ===
    return {
        keyOptions,
        modifierOptions,
        bindings,
        defaultBindings,
        refreshOptions,
        bindDefaults,
        bindHotkey,
        clearHotkey,
      }
})
