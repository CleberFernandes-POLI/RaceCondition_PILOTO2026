#include <zephyr/kernel.h>
#include <zephyr/sys/atomic.h>

#define ITERATIONS 1000
#define STACK_SIZE 1024
#define THREAD_PRIORITY 5

static atomic_t telemetry_packets = ATOMIC_INIT(0);

K_SEM_DEFINE(read_ready, 0, 2);
K_SEM_DEFINE(write_go, 0, 2);
K_SEM_DEFINE(write_done, 0, 2);
K_SEM_DEFINE(round_start, 0, 2);

K_THREAD_STACK_DEFINE(worker_a_stack, STACK_SIZE);
K_THREAD_STACK_DEFINE(worker_b_stack, STACK_SIZE);
static struct k_thread worker_a_data;
static struct k_thread worker_b_data;

static void telemetry_worker(void *arg1, void *arg2, void *arg3)
{
	ARG_UNUSED(arg1);
	ARG_UNUSED(arg2);
	ARG_UNUSED(arg3);

	for (int i = 0; i < ITERATIONS; ++i) {
		k_sem_take(&round_start, K_FOREVER);
		atomic_val_t packets = atomic_get(&telemetry_packets);

		/* Both threads capture the same value before either may write. */
		k_sem_give(&read_ready);
		k_sem_take(&write_go, K_FOREVER);

		/*
		 * The individual atomic access is safe, but this store is not
		 * atomic with the preceding load: the other increment is lost.
		 */
		atomic_set(&telemetry_packets, packets + 1);
		k_sem_give(&write_done);
	}
}

int main(void)
{
	k_thread_create(&worker_a_data, worker_a_stack, STACK_SIZE,
			telemetry_worker, NULL, NULL, NULL,
			THREAD_PRIORITY, 0, K_NO_WAIT);

	k_thread_create(&worker_b_data, worker_b_stack, STACK_SIZE,
			telemetry_worker, NULL, NULL, NULL,
			THREAD_PRIORITY, 0, K_NO_WAIT);

	for (int i = 0; i < ITERATIONS; ++i) {
		k_sem_give(&round_start);
		k_sem_give(&round_start);

		k_sem_take(&read_ready, K_FOREVER);
		k_sem_take(&read_ready, K_FOREVER);

		k_sem_give(&write_go);
		k_sem_give(&write_go);

		k_sem_take(&write_done, K_FOREVER);
		k_sem_take(&write_done, K_FOREVER);
	}

	printk("Pacotes registrados: %d\n",
	       (int)atomic_get(&telemetry_packets));
	printk("Incrementos solicitados: %d\n", 2 * ITERATIONS);
	printk("Atualizacoes perdidas: %d\n",
	       2 * ITERATIONS - (int)atomic_get(&telemetry_packets));

	return 0;
}
