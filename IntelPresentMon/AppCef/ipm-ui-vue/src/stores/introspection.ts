import { ref } from 'vue';
import { defineStore } from 'pinia';
import { Api } from '@/core/api';
import type { Metric } from '@/core/metric';
import type { Stat } from '@/core/stat';
import type { Unit } from '@/core/unit';
import type { Adapter } from '@/core/adapter';
import type { MetricAvailabilityReason } from '@/core/metric-availability-constants';
import {
  getLocalizedMetricAvailabilityDescription,
  getLocalizedMetricText,
  getLocalizedStatText,
} from '@/locales/zh-CN-metrics';

function cloneLocalizedMetric(metric: Metric): Metric {
  const localized = getLocalizedMetricText(metric.id, metric.name);
  return {
    ...metric,
    name: localized?.name ?? metric.name,
    description: localized?.description ?? metric.description,
    availableStatIds: [...metric.availableStatIds],
    deviceAvailability: metric.deviceAvailability.map((entry) => ({ ...entry })),
  };
}

function cloneLocalizedStat(stat: Stat): Stat {
  const localized = getLocalizedStatText(stat.id);
  return {
    ...stat,
    name: localized?.name ?? stat.name,
    description: localized?.description ?? stat.description,
  };
}

function cloneLocalizedAvailabilityReason(
  reason: MetricAvailabilityReason,
): MetricAvailabilityReason {
  return {
    ...reason,
    description:
      getLocalizedMetricAvailabilityDescription(reason.id) ?? reason.description,
  };
}

export const useIntrospectionStore = defineStore('introspection', () => {
  // === State ===
  const metrics = ref<Metric[]>([]);
  const stats = ref<Stat[]>([]);
  const units = ref<Unit[]>([]);
  const adapters = ref<Adapter[]>([]);
  const systemDeviceId = ref<number>(0);
  const metricAvailabilityReasons = ref<MetricAvailabilityReason[]>([]);
  const introspectionLoaded = ref(false);

  // === Actions ===
  async function load() {
    if (introspectionLoaded.value) {
      return;
    }
    const intro = await Api.introspect();
    metrics.value = intro.metrics.map(cloneLocalizedMetric);
    stats.value = intro.stats.map(cloneLocalizedStat);
    units.value = intro.units.map((unit) => ({ ...unit }));
    adapters.value = intro.adapters.map((adapter) => ({ ...adapter }));
    systemDeviceId.value = intro.systemDeviceId;
    metricAvailabilityReasons.value = Array.isArray(intro.metricAvailabilityReasons)
      ? intro.metricAvailabilityReasons.map(cloneLocalizedAvailabilityReason)
      : [];
    introspectionLoaded.value = true;
  }

  // === Exports ===
  return {
    metrics,
    stats,
    units,
    adapters,
    systemDeviceId,
    metricAvailabilityReasons,
    load,
  };
});
